#include "ui.h"
#include "config.h"
#include "sensors.h"
#include <EEPROM.h>

U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);
UIPage currentPage = MENU;

int Kp = DEFAULT_KP;
int Ki = DEFAULT_KI;
int Kd = DEFAULT_KD;
int speedVal = DEFAULT_SPEED;
int analogThr = ANALOG_THR; 
int adsThr = ADS_THR;

unsigned long finalLapTime = 0;
int menuIndex = 0;
const char* menuItems[] = { "RUN BOT", "TUNING", "SENSORS", "CALIB" };

int tuningCursor = 0;
bool isEditing = false;
const char* tuningLabels[] = {"Kp", "Ki", "Kd", "Sp", "ATh", "DTh"};

bool lastUp=HIGH, lastDown=HIGH, lastSel=HIGH, lastBack=LOW;
unsigned long lastPress = 0;
unsigned long lastDrawTime = 0;

// TWEAK: Lower 200 to 100-150 if your button presses feel too slow to register.
const int DEBOUNCE = 200;

void drawMenu(); void drawTuning(); void drawSensors(); void drawReport(); void drawCalibrate();
void beep() { tone(PIN_BUZZER, 2000, 50); }
void setLapTime(unsigned long t) { finalLapTime = t; }

// EEPROM MANAGEMENT: Loads and saves tuning values so they persist after power off.
void loadSettings() {
    if (EEPROM.read(0) != 99) {
        Kp = DEFAULT_KP; Ki = DEFAULT_KI; Kd = DEFAULT_KD; speedVal = DEFAULT_SPEED;
        analogThr = ANALOG_THR; adsThr = ADS_THR;
        saveSettings();
    } else {
        Kp = EEPROM.read(1); Ki = EEPROM.read(2); Kd = EEPROM.read(3); speedVal = EEPROM.read(4);
        EEPROM.get(10, analogThr); EEPROM.get(15, adsThr);
    }
}

void saveSettings() {
    EEPROM.update(0, 99);
    EEPROM.update(1, Kp); EEPROM.update(2, Ki); EEPROM.update(3, Kd); EEPROM.update(4, speedVal);
    EEPROM.put(10, analogThr); EEPROM.put(15, adsThr);
}

// UI INITIALIZATION: Sets up button pins and turns on the OLED display.
void setupUI() {
    pinMode(BTN_UP, INPUT_PULLUP); pinMode(BTN_DOWN, INPUT_PULLUP); pinMode(BTN_SELECT, INPUT_PULLUP);
    u8x8.begin(); u8x8.setFont(u8x8_font_chroma48medium8_r);
    loadSettings();
    drawMenu();
}

// UI STATE MACHINE: Handles button presses, debouncing, and switching screens.
void handleUI() {
    if (currentPage == RUNNING || currentPage == COUNTDOWN) return;

    bool up = digitalRead(BTN_UP);
    bool down = digitalRead(BTN_DOWN);
    bool sel = digitalRead(BTN_SELECT);
    bool back = (analogRead(BTN_BACK_PIN) > 600);

    if (currentPage == CALIBRATE) drawCalibrate(); 

    if (millis() - lastPress < DEBOUNCE) return;

    if (back && !lastBack && currentPage != MENU && currentPage != REPORT) {
        beep(); 
        if (currentPage == TUNING) { saveSettings(); tone(PIN_BUZZER, 1000, 50); }
        currentPage = MENU; drawMenu(); lastPress = millis(); return;
    }

    switch (currentPage) {
        case MENU:
            if (!up && lastUp) { beep(); menuIndex = (menuIndex>0) ? menuIndex-1 : 3; drawMenu(); }
            if (!down && lastDown) { beep(); menuIndex = (menuIndex<3) ? menuIndex+1 : 0; drawMenu(); }
            if (!sel && lastSel) {
                beep();
                if (menuIndex == 0) currentPage = COUNTDOWN; 
                if (menuIndex == 1) { currentPage = TUNING; isEditing=false; drawTuning(); }
                if (menuIndex == 2) { currentPage = SENSORS; u8x8.clear(); }
                if (menuIndex == 3) { currentPage = CALIBRATE; u8x8.clear(); }
            }
            break;

        case TUNING:
            if (!sel && lastSel) { beep(); isEditing = !isEditing; drawTuning(); }
            if (!up && lastUp) {
                if (isEditing) {
                    // TWEAK: Change these numbers (+1, +5, +10) to adjust how fast tuning increments.
                    if (tuningCursor==0) Kp++; else if(tuningCursor==1) Ki++;
                    else if(tuningCursor==2) Kd++; else if(tuningCursor==3) speedVal+=5;
                    else if(tuningCursor==4) analogThr+=10; else adsThr+=100;
                } else if(tuningCursor>0) tuningCursor--;
                drawTuning();
            }
            if (!down && lastDown) {
                if (isEditing) {
                    // TWEAK: Change these numbers (-1, -5, -10) to adjust how fast tuning decrements.
                    if (tuningCursor==0) Kp--; else if(tuningCursor==1) Ki--;
                    else if(tuningCursor==2) Kd--; else if(tuningCursor==3) speedVal-=5;
                    else if(tuningCursor==4) analogThr-=10; else adsThr-=100;
                } else if(tuningCursor<5) tuningCursor++; 
                drawTuning();
            }
            break;

        case SENSORS: drawSensors(); break;
        case CALIBRATE: break; 
        case REPORT: 
            if ((!sel && lastSel) || (!up && lastUp) || (back && !lastBack)) {
                beep(); currentPage = MENU; drawMenu();
            }
            break;
        case COUNTDOWN: case RUNNING: break;
    }
    if (up!=lastUp || down!=lastDown || sel!=lastSel || back!=lastBack) lastPress = millis();
    lastUp=up; lastDown=down; lastSel=sel; lastBack=back;
}

// DRAW FUNCTIONS: Renders the text and menus to the OLED screen.
void drawMenu() {
    u8x8.clear();
    for(int i=0; i<4; i++) {
        if(i==menuIndex) { u8x8.setInverseFont(1); u8x8.drawString(0,i,">"); }
        else u8x8.drawString(0,i," ");
        u8x8.drawString(2,i,menuItems[i]);
        u8x8.setInverseFont(0);
    }
}

void drawTuning() {
    u8x8.clear(); u8x8.drawString(0,0,"TUNING");
    int vals[] = {Kp, Ki, Kd, speedVal, analogThr, adsThr};
    for(int i=0; i<6; i++) {
        if(i==tuningCursor) u8x8.drawString(0,i+1,">");
        u8x8.drawString(2,i+1,tuningLabels[i]); 
        u8x8.setCursor(6,i+1); u8x8.print(vals[i]);
        if(i==tuningCursor && isEditing) u8x8.drawString(13,i+1,"<");
    }
}

void drawCalibrate() {
    u8x8.setCursor(0,2); u8x8.print("CALIB DEPRECATED");
    u8x8.setCursor(0,6); u8x8.print("[BACK] EXIT");
}

void drawReport() {
    u8x8.clear(); u8x8.setFont(u8x8_font_7x14B_1x2_r); 
    u8x8.drawString(0,0,"FINISHED!");
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.drawString(0,3,"TIME:");
    float sec = finalLapTime / 1000.0;
    u8x8.setCursor(6,3); u8x8.print(sec); u8x8.print(" s");
    u8x8.drawString(0,6,"[ANY] EXIT");
}

static const uint8_t CIRCLE_FILLED[] = { 0x3C, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C };
static const uint8_t CIRCLE_EMPTY[] = { 0x3C, 0x42, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C };

extern uint8_t readSensorBits();
extern int calculateError(uint8_t b);

extern int errorDir;
extern int currentPID;
extern int currentLeftPWM;
extern int currentRightPWM;
extern int activeCountDebug;
extern bool checkpointActive;

// LIVE SENSOR DEBUG: Visualizes sensor states, memory direction, and active PID output.
void drawSensors() {
    // TWEAK: Lower 150 to 50-100 if you want the OLED to refresh faster during sensor testing.
    if (millis() - lastDrawTime < 150) return;
    lastDrawTime = millis();
    
    uint8_t bits = readSensorBits();
    int err = calculateError(bits);
    
    int active = 0;
    for(int i=0; i<8; i++) if((bits >> i) & 1) active++;
    activeCountDebug = active;

    bool farLeft = (bits & 0b11100000) > 0;
    bool farRight = (bits & 0b00000111) > 0;
    bool center = (bits & 0b00011000) > 0;

    if (active > 0) {
        if (farLeft && farRight && center) errorDir = 0; 
        else if (farLeft && !farRight) errorDir = -1; 
        else if (farRight && !farLeft) errorDir = 1;  
        else if (center && !farLeft && !farRight) errorDir = 0;
    }

    checkpointActive = (active >= 5); 

    static int debugLastError = 0;
    currentPID = (Kp * err) + (Kd * (err - debugLastError));
    debugLastError = err;
    
    currentLeftPWM = speedVal + currentPID; 
    currentRightPWM = speedVal - currentPID; 
    
    for(int i=0; i<8; i++) {
        if((bits>>(7-i)) & 1) u8x8.drawTile(i*2, 1, 1, (uint8_t*)CIRCLE_FILLED);
        else u8x8.drawTile(i*2, 1, 1, (uint8_t*)CIRCLE_EMPTY);
    }
    
    u8x8.setCursor(0, 3);
    u8x8.print("Err:"); u8x8.print(err); u8x8.print(" Act:"); u8x8.print(activeCountDebug); u8x8.print(" ");
    
    u8x8.setCursor(0, 4);
    u8x8.print("Mem:"); 
    if (errorDir == 1) u8x8.print("R ");
    else if (errorDir == -1) u8x8.print("L ");
    else u8x8.print("0 ");
    u8x8.print(" CHK:"); 
    if (checkpointActive) u8x8.print("YES"); else u8x8.print("NO ");

    u8x8.setCursor(0, 5);
    u8x8.print("PID: "); u8x8.print(currentPID); u8x8.print("   ");
    
    u8x8.setCursor(0, 6); 
    u8x8.print("L:"); u8x8.print(currentLeftPWM); 
    u8x8.print(" R:"); u8x8.print(currentRightPWM); u8x8.print("  ");
}

int getKp() { return Kp; } int getKi() { return Ki; } int getKd() { return Kd; } int getSpeed() { return speedVal; }
int getAnalogThr() { return analogThr; } int getAdsThr() { return adsThr; }