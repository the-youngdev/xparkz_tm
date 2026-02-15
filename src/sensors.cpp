#include "sensors.h"
#include "config.h"
#include "ui.h" 
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;
bool inverted = false; 

// TWEAK: Change to 'false' if your robot steers in the opposite direction it is supposed to.
bool FLIP_SENSOR_ARRAY = true; 

void setupSensors() {
    ads.begin();
    ads.setGain(GAIN_ONE);
    ads.setDataRate(RATE_ADS1115_860SPS); 
}

uint8_t readSensorBits() {
    uint8_t bits = 0;
    int raw[8];
    
    // SENSOR POLLING: Read from Nano analog pins and ADS1115 ADC.
    raw[0] = analogRead(SENS_NANO_L1); raw[1] = analogRead(SENS_NANO_L2);
    raw[2] = ads.readADC_SingleEnded(0); raw[3] = ads.readADC_SingleEnded(1);
    raw[4] = ads.readADC_SingleEnded(2); raw[5] = ads.readADC_SingleEnded(3);
    raw[6] = analogRead(SENS_NANO_R1); raw[7] = analogRead(SENS_NANO_R2);

    int aThr = getAnalogThr(); 
    int dThr = getAdsThr();    

    // THRESHOLDING: Convert raw analog readings into a clean 8-bit binary representation.
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
        if (raw[7] > aThr) bits |= (1<<7); 
        if (raw[6] > aThr) bits |= (1<<6); 
        if (raw[5] > dThr) bits |= (1<<5); 
        if (raw[4] > dThr) bits |= (1<<4); 
        if (raw[3] > dThr) bits |= (1<<3); 
        if (raw[2] > dThr) bits |= (1<<2); 
        if (raw[1] > aThr) bits |= (1<<1); 
        if (raw[0] > aThr) bits |= (1<<0); 
    }

    // TURN-SAFE INVERTER: Automatically flips background color ONLY if the line is perfectly centered.
    if (bits == 0b00011000 || bits == 0b00111100 || bits == 0b00001100 || 
        bits == 0b00110000 || bits == 0b00000110 || bits == 0b01100000 ||
        bits == 0b00010000 || bits == 0b00001000 || bits == 0b00011100 || bits == 0b00111000) {
        inverted = false; 
    }
    else if (bits == 0b11100111 || bits == 0b11000011 || bits == 0b11110011 || 
             bits == 0b11001111 || bits == 0b11111001 || bits == 0b10011111 ||
             bits == 0b11101111 || bits == 0b11110111 || bits == 0b11100011 || bits == 0b11000111) {
        inverted = true;
    }

    // INVERSION APPLICATION: Ensures the rest of the code always sees '1' as the line.
    if (inverted) bits = ~bits; 
    return bits;
}

int calculateError(uint8_t bits) {
    if (bits == 0) return -999; 
    if (bits == 0xFF) return 999; 
    
    long num = 0, den = 0;
    
    // TWEAK: Increase the outer numbers (e.g., 40 to 50 or 60) for sharper, more aggressive PID steering.
    int weights[] = {-40, -30, -20, -10, 10, 20, 30, 40};

    // ERROR CALCULATION: Averages the active weights to find the exact center of the line.
    for(int i=0; i<8; i++) {
        if ((bits >> (7-i)) & 1) { num += weights[i]; den++; }
    }
    if (den == 0) return 0;
    return num / den;
}

// HELPER FUNCTIONS: Quick checks for gaps, stops, and OLED display data.
bool isLineLost(uint8_t bits) { return (bits == 0); }
bool isStopBox(uint8_t bits) { return (bits == 0xFF); }
String getLineColor() { return inverted ? "WHT" : "BLK"; }