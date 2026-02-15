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
    handleUI();

    if (currentPage == COUNTDOWN) {
        for(int i=3; i>0; i--) { drawCountNum(i); tone(PIN_BUZZER, 2000, 100); delay(1000); }
        drawCountNum(0); u8x8.clear(); u8x8.drawString(0,0,"RUNNING...");
        tone(PIN_BUZZER, 3000, 500); startTime = millis(); currentPage = RUNNING; 
    }

    if (currentPage == RUNNING) {
        if (analogRead(BTN_BACK_PIN) > 600) {
            stopMotors(); setLapTime(millis() - startTime);
            tone(PIN_BUZZER, 1000, 100); currentPage = REPORT; drawReport(); return;
        }

        uint8_t bits = readSensorBits();
        int error = calculateError(bits);
        int activeCount = 0;
        for(int i=0; i<8; i++) if ((bits >> i) & 1) activeCount++;
        activeCountDebug = activeCount; 

        // 1. UNIFIED 80ms STOP BOX DETECTION
        if (activeCount >= 7) { 
             if (stopBoxTimer == 0) stopBoxTimer = millis();
             if (millis() - stopBoxTimer > 80) { 
                brakeMotors(); delay(100); stopMotors(); 
                setLapTime(millis() - startTime);
                tone(PIN_BUZZER, 4000, 800); currentPage = REPORT; drawReport(); return;
             }
        } else { stopBoxTimer = 0; }

        // 2. CHECKPOINT DETECTION
        if (activeCount >= 5) { 
            lastCrossroadTime = millis(); 
            checkpointActive = true;
            digitalWrite(PIN_LED, HIGH); 
        }

        // 3. AGGRESSIVE MEMORY 
        int leftW = ((bits >> 7) & 1) + ((bits >> 6) & 1) + ((bits >> 5) & 1);
        int rightW = ((bits >> 2) & 1) + ((bits >> 1) & 1) + ((bits >> 0) & 1);
        if (bits != 0 && activeCount <= 4) {
            if (leftW > rightW) errorDir = -1;
            else if (rightW > leftW) errorDir = 1;
        }

        // ==========================================
        // 4. FORCED TRUE SPIN (Acute Angle Fix)
        // ==========================================
        if (bits == 0) { 
            int pivotPower = 255; 
            if (errorDir == 0) {
                if (lastError > 0) errorDir = 1;
                else if (lastError < 0) errorDir = -1;
            }

            if (errorDir != 0) { 
                brakeMotors(); delay(20);
                
                // EXPLICIT REVERSE COMMANDS: One wheel forward (+), one wheel backward (-)
                if (errorDir == 1) { 
                    moveStraight(pivotPower, -pivotPower); delay(40); 
                    while((readSensorBits() & 0b00111100) == 0) { 
                        moveStraight(pivotPower, -pivotPower); 
                    }
                } else { 
                    moveStraight(-pivotPower, pivotPower); delay(40);
                    while((readSensorBits() & 0b00111100) == 0) { 
                        moveStraight(-pivotPower, pivotPower); 
                    }
                }
                
                brakeMotors(); delay(60); 
                lastError = 0; errorDir = 0; 
                return;
            }
            moveStraight(getSpeed(), getSpeed());
            return; 
        }

        // 5. FULL SPEED GLIDE
        if (millis() - lastCrossroadTime < 30) {
            moveStraight(getSpeed(), getSpeed()); 
            return; 
        } else {
            checkpointActive = false;
            digitalWrite(PIN_LED, LOW);
        }

        // ==========================================
        // 6. DYNAMIC PID (The "Inner Wheel Reverse" Fix)
        // ==========================================
        P = error;
        int currentDiff = error - lastError;

        if (currentDiff != 0) {
            delay(2); 
        }
        
        D = currentDiff;
        lastError = error;

        currentPID = (getKp() * P) + (getKd() * D);
        
        // CRITICAL FIX: Allow deep reverse on sharp curves
        if (abs(error) > 15) {
            // High error = Sharp Turn. Allow PID to max out so inner wheel goes negative (reverse)
            currentPID = constrain(currentPID, -255, 255);
        } else {
            // Low error = Straightaway. Clamp to base speed to prevent straight-line jitter
            currentPID = constrain(currentPID, -getSpeed(), getSpeed());
        }
        
        currentLeftPWM = getSpeed() + currentPID; 
        currentRightPWM = getSpeed() - currentPID;

        // Tight loop dampener
        if (abs(error) > 20) {
            currentLeftPWM *= 0.70;
            currentRightPWM *= 0.70;
        }
        
        // Because your motors.cpp moveStraight() handles negatives beautifully, 
        // the inner wheel will automatically reverse itself here.
        moveStraight(currentLeftPWM, currentRightPWM);
    }
}