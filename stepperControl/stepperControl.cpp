// stepperControl.cpp - Stepper motor control logic implementation
#include "stepperControl.h"

// State and variables
enum State motorState = IDLE;
bool startCushion = false;
bool endCushion = false;
int baseDelay = (MIN_PULSE * 2 * 200 / PPR * 100 / SPEED * RDNY);
int stepDelay = baseDelay;
int orderInput = 0, responseInput = 0, differential = 0, startPos = 0, endPos = 0;

// Stub functions for hardware I/O (replace with actual implementation)
void digitalWrite(int pin, bool value) {
    // TODO: Implement hardware-specific digital output
}

void pinMode(int pin, int mode) {
    // TODO: Implement hardware-specific pin mode setup
}

int analogRead(int pin) {
    // TODO: Implement hardware-specific analog input
    return 512; // Placeholder
}

void delayMicroseconds(int us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

// Helper functions
void runStepper(int stepDelay) {
    if (stepDelay <= MIN_PULSE)
        stepDelay = baseDelay;
    digitalWrite(PUL_PIN, true);
    delayMicroseconds(stepDelay);
    digitalWrite(PUL_PIN, false);
    delayMicroseconds(stepDelay);
}

void setCushion() {
    startCushion = std::abs(responseInput - startPos) <= CUSHION;
    endCushion = std::abs(responseInput - endPos) <= CUSHION;
    if (motorState == IDLE) {
        startCushion = false;
        endCushion = false;
    }
}

void updateInputs() {
    orderInput = analogRead(ORDER_PIN);
    responseInput = analogRead(RESPONSE_PIN);
    differential = orderInput - responseInput;
}

void updateState() {
    if (differential > DEADBAND) {
        motorState = CW;
        setCushion();
    } else if (differential < -DEADBAND) {
        motorState = CCW;
        setCushion();
    } else {
        if (std::abs(differential) <= PRECISION)
            motorState = IDLE;
        startPos = responseInput;
        endPos = orderInput;
    }
}

void updateOutputs() {
    digitalWrite(DIR_PIN, motorState == CW);
    if (motorState == IDLE) {
        digitalWrite(EN_PIN, true); // Disable driver
        stepDelay = baseDelay;
    } else {
        digitalWrite(EN_PIN, false); // Enable driver
        if (startCushion)
            stepDelay -= ACCELERATION;
        else if (endCushion)
            stepDelay += ACCELERATION;
        runStepper(stepDelay);
    }
}

void runSerial() {
    std::cout << "Order: " << orderInput
              << "\tResponse: " << responseInput
              << "\tDiff: " << differential
              << "\tState: ";
    switch (motorState) {
        case IDLE: std::cout << "IDLE"; break;
        case CW: std::cout << "CW"; break;
        case CCW: std::cout << "CCW"; break;
    }
    std::cout << std::endl;
}

// Arduino-style setup and loop wrappers
void stepperSetup() {
    pinMode(DIR_PIN, 1); // OUTPUT
    pinMode(PUL_PIN, 1);
    pinMode(EN_PIN, 1);
}

void stepperLoop() {
    updateInputs();
    updateState();
    updateOutputs();
    // runSerial(); // Uncomment for debug output
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}
