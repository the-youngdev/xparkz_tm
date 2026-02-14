#include "sensors.h"
#include "config.h"
#include "ui.h" 
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;
bool inverted = false; 

// ==========================================
// 🔄 HARDWARE FLIP SWITCH
// ==========================================
bool FLIP_SENSOR_ARRAY = true; 
// ==========================================

void setupSensors() {
    ads.begin();
    ads.setGain(GAIN_ONE);
    ads.setDataRate(RATE_ADS1115_860SPS); 
}

uint8_t readSensorBits() {
    uint8_t bits = 0;
    int raw[8];
    
    raw[0] = analogRead(SENS_NANO_L1); raw[1] = analogRead(SENS_NANO_L2);
    raw[2] = ads.readADC_SingleEnded(0); raw[3] = ads.readADC_SingleEnded(1);
    raw[4] = ads.readADC_SingleEnded(2); raw[5] = ads.readADC_SingleEnded(3);
    raw[6] = analogRead(SENS_NANO_R1); raw[7] = analogRead(SENS_NANO_R2);

    int aThr = getAnalogThr(); 
    int dThr = getAdsThr();    

    if (!FLIP_SENSOR_ARRAY) {
        if (raw[0] > aThr) bits |= (1<<7);
        if (raw[1] > aThr) bits |= (1<<6);
        if (raw[2] > dThr) bits |= (1<<5);
        if (raw[3] > dThr) bits |= (1<<4);
        if (raw[4] > dThr) bits |= (1<<3);
        if (raw[5] > dThr) bits |= (1<<2);
        if (raw[6] > aThr) bits |= (1<<1);
        if (raw[7] > aThr) bits |= (1<<0);
    } else {
        // Upside-down Array Fix
        if (raw[7] > aThr) bits |= (1<<7); 
        if (raw[6] > aThr) bits |= (1<<6); 
        if (raw[5] > dThr) bits |= (1<<5); 
        if (raw[4] > dThr) bits |= (1<<4); 
        if (raw[3] > dThr) bits |= (1<<3); 
        if (raw[2] > dThr) bits |= (1<<2); 
        if (raw[1] > aThr) bits |= (1<<1); 
        if (raw[0] > aThr) bits |= (1<<0); 
    }

    // ==========================================
    // 🔄 RESTORED: YOUR EXACT ORIGINAL AUTO-INVERTER
    // ==========================================
    if (bits == 0b01100000 || bits == 0b01110000 || bits == 0b00100000 ||
        bits == 0b00110000 || bits == 0b00111000 || bits == 0b00010000 ||
        bits == 0b00011000 || bits == 0b00011100 || bits == 0b00001000 ||
        bits == 0b00001100 || bits == 0b00001110 || bits == 0b00000100 ||
        bits == 0b00000110) inverted = false;

    else if (bits == 0b10011111 || bits == 0b10001111 || bits == 0b11011111 ||
             bits == 0b11001111 || bits == 0b11000111 || bits == 0b11101111 ||
             bits == 0b11100111 || bits == 0b11100011 || bits == 0b11110111 ||
             bits == 0b11110011 || bits == 0b11110001 || bits == 0b11111011 ||
             bits == 0b11111001) inverted = true;

    if (inverted) bits = ~bits; 
    return bits;
}

int calculateError(uint8_t bits) {
    if (bits == 0) return -999; 
    if (bits == 0xFF) return 999; 
    
    long num = 0, den = 0;
    int weights[] = {-40, -30, -20, -10, 10, 20, 30, 40};

    for(int i=0; i<8; i++) {
        if ((bits >> (7-i)) & 1) { num += weights[i]; den++; }
    }
    if (den == 0) return 0;
    return num / den;
}

bool isLineLost(uint8_t bits) { return (bits == 0); }
bool isStopBox(uint8_t bits) { return (bits == 0xFF); }
String getLineColor() { return inverted ? "WHT" : "BLK"; }