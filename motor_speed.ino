/*   
 *   Basic example code for controlling a stepper with the AccelStepper library
 *   Revised 2025/jul/29, by Mikhail R, to allow bi-directional output
 *      
 *   by Dejan, https://howtomechatronics.com
 */

#include <AccelStepper.h>
#define MIDPOINT 3*1024/5 // 1-3V CW, 3-5V CCW

// Define the stepper motor and the pins that is connected to
AccelStepper stepper1(1, 8, 10); // (Type of driver: with 2 pins, STEP, DIR)

void setup() {
  // Set maximum speed value for the stepper
  stepper1.setMaxSpeed(1000);
}

void loop() {
  stepper1.setSpeed( analogRead(A6) - MIDPOINT );
  // Step the motor with a constant speed previously set by setSpeed();
  stepper1.runSpeed();
}
