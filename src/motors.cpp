#include "motors.h"
#include "config.h"
#include <Arduino.h>

void setupMotors() {
    pinMode(MOT_L_PWM, OUTPUT); pinMode(MOT_L_IN1, OUTPUT); pinMode(MOT_L_IN2, OUTPUT);
    pinMode(MOT_R_PWM, OUTPUT); pinMode(MOT_R_IN1, OUTPUT); pinMode(MOT_R_IN2, OUTPUT);
    pinMode(MOT_STBY, OUTPUT); digitalWrite(MOT_STBY, HIGH); 
}

void moveStraight(int left, int right) {
    left = constrain(left, -255, 255);
    right = constrain(right, -255, 255);

    if (left >= 0) { digitalWrite(MOT_L_IN1, HIGH); digitalWrite(MOT_L_IN2, LOW); }
    else { digitalWrite(MOT_L_IN1, LOW); digitalWrite(MOT_L_IN2, HIGH); }
    analogWrite(MOT_L_PWM, abs(left));

    if (right >= 0) { digitalWrite(MOT_R_IN1, HIGH); digitalWrite(MOT_R_IN2, LOW); }
    else { digitalWrite(MOT_R_IN1, LOW); digitalWrite(MOT_R_IN2, HIGH); }
    analogWrite(MOT_R_PWM, abs(right));
}

void turnCCW(int speed) { 
    digitalWrite(MOT_L_IN1, LOW); digitalWrite(MOT_L_IN2, HIGH);
    digitalWrite(MOT_R_IN1, HIGH); digitalWrite(MOT_R_IN2, LOW);
    analogWrite(MOT_L_PWM, speed); analogWrite(MOT_R_PWM, speed);
}

void turnCW(int speed) { 
    digitalWrite(MOT_L_IN1, HIGH); digitalWrite(MOT_L_IN2, LOW);
    digitalWrite(MOT_R_IN1, LOW); digitalWrite(MOT_R_IN2, HIGH);
    analogWrite(MOT_L_PWM, speed); analogWrite(MOT_R_PWM, speed);
}

void stopMotors() {
    analogWrite(MOT_L_PWM, 0); analogWrite(MOT_R_PWM, 0);
}

void brakeMotors() {
    digitalWrite(MOT_L_IN1, HIGH); digitalWrite(MOT_L_IN2, HIGH);
    digitalWrite(MOT_R_IN1, HIGH); digitalWrite(MOT_R_IN2, HIGH);
    // CRITICAL FIX: Force PWM to 255 to lock the H-Bridge
    analogWrite(MOT_L_PWM, 255); analogWrite(MOT_R_PWM, 255); 
}