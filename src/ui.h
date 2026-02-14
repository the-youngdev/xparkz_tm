#pragma once
#include <Arduino.h>
#include <U8g2lib.h>

extern U8X8_SH1106_128X64_NONAME_HW_I2C u8x8;

// Added CALIBRATE to the pages
enum UIPage { MENU, COUNTDOWN, RUNNING, TUNING, SENSORS, REPORT, CALIBRATE };
extern UIPage currentPage; 

void setupUI();
void handleUI();
void drawMenu();

void loadSettings();
void saveSettings();

void setLapTime(unsigned long t);
void drawReport(); 

// Getters
int getKp();
int getKi();
int getKd();
int getSpeed();
int getAnalogThr();
int getAdsThr();