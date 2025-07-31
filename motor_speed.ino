/*   
 *   Basic example code for controlling a stepper with the AccelStepper library
 *   Revised 2025/jul/29, by Mikhail R, to allow bi-directional output
 *   Revised 2025/jul/30, by Mikhail R, for bipolar driver, and using PPR to set max rotation speed at 1rev/s
 *   Revised 2025/jul/31, by Mikhail R, returning to DRIVER mode, cuz bipolar/4wire setup is for no driver
 *      
 *   by Dejan, https://howtomechatronics.com
 *//////////////////////////////////////////////////////////////////////////////

#include <AccelStepper.h>
// pins
#define PUL_PIN 8
#define DIR_PIN 10
#define ORDER_PIN A6 
// numeric constants for fine-tuning
#define MIDPOINT 3*1024/5 // 1-3V CW, 3-5V CCW
#define PPR 400 // Steps per revolution (e.g. 200 for full-step, 400 for half-step)
#define MIN_STEP 1000 // in µs, eg. 1000us = a 2ms period, or 500Hz MaxSpeed
// Construct stepper motor
AccelStepper stepper1(AccelStepper::DRIVER, PUL_PIN, DIR_PIN); 

void setup() {
  stepper1.setMaxSpeed(PPR); // Sets maximum rotation speed at 1rev/s
  stepper1.setMinPulseWidth(MIN_STEP); // otherwise step size is way too small
}

void loop() {
  stepper1.setSpeed( analogRead(ORDER_PIN) - MIDPOINT );
  stepper1.runSpeed(); // Step the motor with a constant speed previously set by setSpeed();
}

