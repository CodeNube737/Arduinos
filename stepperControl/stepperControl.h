// stepperControl.h - Stepper motor control logic header
#pragma once

#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>

// Pin definitions (replace with actual hardware interface as needed)
constexpr int PUL_PIN = 8;
constexpr int DIR_PIN = 10;
constexpr int EN_PIN = 12;
constexpr int ORDER_PIN = 6;     // Analog input stub
constexpr int RESPONSE_PIN = 7;  // Analog input stub

// Numeric constants for fine-tuning
constexpr int PPR = 400;
constexpr int MIN_PULSE = 1250; // in microseconds
constexpr int SPEED = 90;
constexpr int ACCELERATION = 1;
constexpr int PRECISION = 2;
constexpr int DEADBAND = 16;
constexpr int CUSHION = 30;
constexpr int RDNY = 10;
constexpr int MINinput = 204;
constexpr int MAXinput = 1023;

// State enum
enum State { IDLE, CW, CCW };
extern State motorState;
extern bool startCushion;
extern bool endCushion;
extern int baseDelay;
extern int stepDelay;
extern int orderInput, responseInput, differential, startPos, endPos;

// Hardware abstraction (to be implemented for platform)
void digitalWrite(int pin, bool value);
void pinMode(int pin, int mode);
int analogRead(int pin);
void delayMicroseconds(int us);

// Stepper logic functions
void runStepper(int stepDelay);
void setCushion();
void updateInputs();
void updateState();
void updateOutputs();
void runSerial();
void stepperSetup();
void stepperLoop();
