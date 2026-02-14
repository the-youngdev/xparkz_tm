#pragma once

void setupMotors();
void moveStraight(int leftMotorSpeed, int rightMotorSpeed);
void turnCCW(int speed);
void turnCW(int speed);
void stopMotors();
void brakeMotors();