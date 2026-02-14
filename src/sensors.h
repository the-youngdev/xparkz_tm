#pragma once
#include <Arduino.h>

void setupSensors();
uint8_t readSensorBits(); 
int calculateError(uint8_t bits);
bool isOutOfLine(uint8_t sensorReadings); // YOUR function restored
bool isStopBox(uint8_t bits);
String getLineColor();