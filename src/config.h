#pragma once
#include <Arduino.h>

// ================= HARDWARE PINS =================
// Buttons
#define BTN_UP      2   
#define BTN_DOWN    3   
#define BTN_SELECT  12  
#define BTN_BACK_PIN A6 

// Motors (TB6612)
#define MOT_L_PWM   5
#define MOT_L_IN1   4
#define MOT_L_IN2   7
#define MOT_R_PWM   6
#define MOT_R_IN1   8
#define MOT_R_IN2   9
#define MOT_STBY    10  

#define PIN_BUZZER  11
#define PIN_LED     13

// Sensors
#define SENS_NANO_L1 A0 
#define SENS_NANO_L2 A1 
#define SENS_NANO_R1 A2 
#define SENS_NANO_R2 A3 

// ================= TUNING (From Working Code) =================
// Thresholds (0 = White, 1 = Black)
#define ANALOG_THR 200     
#define ADS_THR    8000  

// Default PID
#define DEFAULT_SPEED 140
#define DEFAULT_KP 25      
#define DEFAULT_KD 80     
#define DEFAULT_KI 0

// Logic Constants
#define OUT_OF_LINE 20