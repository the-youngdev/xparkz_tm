#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "sensors.h"
#include "motors.h" 
#include "ui.h"

int lastError = 0;
int errorDir = 0; 
unsigned long startTime = 0;
int currentPID = 0, currentLeftPWM = 0, currentRightPWM = 0, activeCountDebug = 0; 
unsigned long stopBoxTimer = 0, lastCrossroadTime = 0; 
bool checkpointActive = false; 

int P = 0;
int D = 0;

void setup() {
    Wire.begin(); Wire.setClock(400000); 
    setupSensors(); setupMotors(); setupUI();
    pinMode(PIN_BUZZER, OUTPUT); pinMode(PIN_LED, OUTPUT); 
    for(int i=0; i<5; i++) { digitalWrite(PIN_LED, HIGH); delay(50); digitalWrite(PIN_LED, LOW); delay(50); }
    tone(PIN_BUZZER, 2000, 100); delay(150); tone(PIN_BUZZER, 3000, 200); 
}

void drawCountNum(int n) {
    u8x8.clear(); u8x8.setFont(u8x8_font_profont29_2x3_n);
    u8x8.setCursor(6,2); u8x8.print(n); u8x8.setFont(u8x8_font_chroma48medium8_r);
}

void loop() {
    // UPDATES OLED DISPLAY AND MENU NAVIGATION
    handleUI();

    if (currentPage == COUNTDOWN) {
        for(int i=3; i>0; i--) { drawCountNum(i); tone(PIN_BUZZER, 2000, 100); delay(1000); }
        drawCountNum(0); u8x8.clear(); u8x8.drawString(0,0,"RUNNING...");
        tone(PIN_BUZZER, 3000, 500); startTime = millis(); currentPage = RUNNING; 
    }

    if (currentPage == RUNNING) {
        // EMERGENCY STOP BUTTON
        if (analogRead(BTN_BACK_PIN) > 600) {
            stopMotors(); setLapTime(millis() - startTime);
            tone(PIN_BUZZER, 1000, 100); currentPage = REPORT; drawReport(); return;
        }

        // READ SENSORS AND CALCULATE CURRENT ERROR
        uint8_t bits = readSensorBits();
        int error = calculateError(bits);
        int activeCount = 0;
        for(int i=0; i<8; i++) if ((bits >> i) & 1) activeCount++;
        activeCountDebug = activeCount; 

        // 1. STOP BOX: Detects the finish box and stops the robot.
        if (activeCount >= 7) { 
             if (stopBoxTimer == 0) stopBoxTimer = millis();
             // TWEAK: Lower 80 to 40-50 if it ignores the finish box at high speeds
             if (millis() - stopBoxTimer > 80) { 
                brakeMotors(); delay(100); stopMotors(); 
                setLapTime(millis() - startTime);
                tone(PIN_BUZZER, 4000, 800); currentPage = REPORT; drawReport(); return;
             }
             moveStraight(getSpeed(), getSpeed());
             return; 
        } else { 
             stopBoxTimer = 0; 
        }

        // 2. CHECKPOINT: Detects horizontal intersections to start the glide timer.
        if (activeCount >= 5) { 
            lastCrossroadTime = millis(); 
            checkpointActive = true;
            digitalWrite(PIN_LED, HIGH); 
        }

        // 3. MEMORY: Logs the last known line position (left or right) before it vanishes.
        int leftW = ((bits >> 7) & 1) + ((bits >> 6) & 1) + ((bits >> 5) & 1);
        int rightW = ((bits >> 2) & 1) + ((bits >> 1) & 1) + ((bits >> 0) & 1);
        if (bits != 0 && activeCount <= 4) {
            if (leftW > rightW) errorDir = -1;
            else if (rightW > leftW) errorDir = 1;
        }

        // 4.Executes a zero-radius turn when the line is completely lost.
        if (bits == 0) { 
            int pivotPower = 255; 
            if (errorDir == 0) {
                // TWEAK: Increase 5 to 10-15 if it falsely spins on straight gaps
                if (lastError > 5) errorDir = 1;
                else if (lastError < -5) errorDir = -1;
                else if (millis() - lastCrossroadTime < 150) {
                    errorDir = -1; 
                }
            }

            if (errorDir != 0) { 
                brakeMotors(); 
                // TWEAK: Increase 50 to 60-70 if the bot skids forward before spinning
                delay(50); 
                
                if (errorDir == 1) { 
                    moveStraight(pivotPower, -pivotPower); delay(40); 
                    while((readSensorBits() & 0b00111100) == 0) { moveStraight(pivotPower, -pivotPower); }
                } else { 
                    moveStraight(-pivotPower, pivotPower); delay(40);
                    while((readSensorBits() & 0b00111100) == 0) { moveStraight(-pivotPower, pivotPower); }
                }
                
                brakeMotors(); delay(60); 
                lastError = 0; errorDir = 0; 
                return;
            }
            moveStraight(getSpeed(), getSpeed());
            return; 
        }

        // 5.Ignores sensor data briefly to skip over horizontal intersections.
        // TWEAK: Lower 30 to 15-20 if it overshoots the line after passing a crossroad
        if (millis() - lastCrossroadTime < 30) {
            moveStraight(getSpeed(), getSpeed()); 
            return; 
        } else {
            checkpointActive = false;
            digitalWrite(PIN_LED, LOW);
        }

        // 6. PID STEERING: Calculates the motor adjustment based on current and past error.
        P = error;
        int currentDiff = error - lastError;

        if (currentDiff != 0) delay(2); 
        
        D = currentDiff;
        lastError = error;

        currentPID = (getKp() * P) + (getKd() * D);
        
        if (abs(error) > 15) currentPID = constrain(currentPID, -255, 255);
        else currentPID = constrain(currentPID, -getSpeed(), getSpeed());
        
        currentLeftPWM = getSpeed() + currentPID; 
        currentRightPWM = getSpeed() - currentPID;
 
        if (abs(error) > 20) {
            // TWEAK: Lower 0.85 to 0.60-0.70 if the bot flies off the track on tight loops
            currentLeftPWM *= 0.60;
            currentRightPWM *= 0.60;
        }
        
        moveStraight(currentLeftPWM, currentRightPWM);
    }
}