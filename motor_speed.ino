/*   
 *   Basic example code for controlling a stepper with the AccelStepper library
 *   Revised 2025/jul/29, by Mikhail R, to allow bi-directional output
 *   Revised 2025/jul/30, by Mikhail R, for bipolar driver, and using PPR to set max rotation speed at 1Hz
 *      
 *   by Dejan, https://howtomechatronics.com
 *//////////////////////////////////////////////////////////////////////////////

#include <AccelStepper.h>
// pins
#define PUL_PIN_pos 8
#define PUL_PIN_neg 9
#define DIR_PIN_pos 10
#define DIR_PIN_neg 11
#define ORDER_PIN A6 
// numeric constants for fine-tuning
#define MIDPOINT 3*1024/5 // 1-3V CW, 3-5V CCW
#define PPR 400 // Steps per revolution (e.g. 200 for full-step, 400 for half-step)
// Construct stepper motor
AccelStepper stepper1(AccelStepper::FULL4WIRE, PUL_PIN_pos, PUL_PIN_neg, DIR_PIN_pos, DIR_PIN_neg); 

void setup() {
  stepper1.setMaxSpeed(PPR); // Set maximum speed value for the stepper
}

void loop() {
  stepper1.setSpeed( analogRead(ORDER_PIN) - MIDPOINT );
  stepper1.runSpeed(); // Step the motor with a constant speed previously set by setSpeed();
}

