// stepperControl.ino
/* By Mikhail R, 2025/aug/1
 *    Runs a stepper motor through a typical driver with DIR, PUL, and EN inputs and feedback
 * Revised By MR 2025/aug/5, made serial better, and fixed improper CW/CW set. 
 *    Also, constrained input to prevent URV/LRV failure. And ensured motor works as intended at 800 PPR, and settings below.
 *    note: best debug is with serial on, or serial off and 2sec/div 'scope on RESPONSE_PIN
 * Revised By MR 2025/aug/6, adding reverse mode, and changing some of the comments and structure
*/
////////////////////////////////////////////////////////////////////////

// pins
#define PUL_PIN 8
#define DIR_PIN 10
#define EN_PIN 12
#define ORDER_PIN A6     // system input
#define RESPONSE_PIN A7  // motor feedback
// numeric constants for fine-tuning
#define PPR 800            // created because Leadshine ctrllr sets ppr. 200 = full, 400 = 1/2... etc. Adjust MIN_PULSE with PPR.
#define MIN_PULSE 625      // in µs, eg. 625us = a 1.25ms period, or 800Hz base-speed, at 800 ppr, that's 1rev/sec. Adjust MIN_PULSE with PPR.
#define MAX_PULSE 5000     // slowest speed
#define SPEED 110          // percentage of RDNY-speed
#define ACCELERATION 30    // for CUSHION, must be positive. e.g. 5*30=150µs gained from acceleration
#define PRECISION 5        // as small an integer as possible but can still read feedback (response)
#define DEADBAND 32        // should be at least enough to cover noise, for exiting IDLE. must be larger at smaller PPR.
#define CUSHION 400        // as big as desired, speed-up/slow-down before leaving/entering IDLE
#define RDNY 10            // seconds per revolution (anticipated speed)
#define serialMode false   // enables USB com of ORDER, RESPONSE, DIFF, but disables motor's EN pin
#define reverseMode false  // (true/false) flips the direction of the motor vs input knob
// global variables
enum State { IDLE,
             CW,
             CCW };
State motorState = IDLE;
bool startCushion = false;                                                          // a sort-of sub state flag
bool endCushion = false;                                                            // a sort-of sub state flag
int orderInput = 0, responseInput = 0, differential = 0, startPos = 0, endPos = 0;  // should never exceed ±1023, or ±5V
const int MINinput = 204 + PRECISION;                                               // 1 Volt (4mA)
const int MAXinput = 1023 - 67 - PRECISION;                                         // 5 Volts-ish (20mA)... don't ask about the 67, that's from experiment
const double baseDelay = (MIN_PULSE * 200 / PPR * 100 / SPEED * RDNY);              // see notes
double stepDelay = baseDelay;                                                       // starting value
//int counter = 0; // may help for future iterations?

// helper functions

void runStepper(int stepSize) {
  constrain(stepSize, baseDelay, MAX_PULSE);  // set minimum and maximum speeds for the stepper
  digitalWrite(PUL_PIN, HIGH);
  delayMicroseconds(stepSize);  // this is the briefer of the two
  digitalWrite(PUL_PIN, LOW);
  delayMicroseconds(stepSize);
  return;
}

void setCushion() {  // set flags (sub-state)
  startCushion = abs(responseInput - startPos) <= CUSHION;
  endCushion = abs(responseInput - endPos) <= CUSHION;
  if (motorState == IDLE) {  // IDLE state should turn-off all cushion/accel flags
    startCushion = false;
    endCushion = false;
  }
  return;
}

int constrainInput(int value) {
  return constrain(value, MINinput, MAXinput);
}

void calculteDifferential() {  // motor turns with/counter control-knob's direction. Must be run before updateState()
  if (reverseMode) {
    differential = orderInput - responseInput;
  } else {
    responseInput = MAXinput - responseInput + MINinput;  // you can draw a truth table or just run serialMode to understand
    differential = responseInput - orderInput;            // the mid-point should be about 580 counts
  }
  return;
}

void updateInputs() {  // updateInputs() and subtract to get int differential
  orderInput = constrainInput(analogRead(ORDER_PIN));
  responseInput = constrainInput(analogRead(RESPONSE_PIN));
  calculteDifferential();  // Set motor direction, must be run before updateState()
  // float orderVoltage = orderInput*5.0/ 1023.0;
  // float responseVoltage = responseInput*5.0 /1023.0;
  return;
}

void updateState() {  // updateState() based solely on differential.
  /* MUST: 
   * accelerate motor, 
   * reach speed (don't get trapped in loops, or u may miss the target), 
   * coast below max/base speed, 
   * slow down on approach,
   * stop at target, set by ORDER
   * complete min-point to maxpoint (0-1023) in about 10 seconds (RDNY)
  */
  // CW
  if (differential > DEADBAND) {  // typical deadband to exit IDLE state
    motorState = CCW;
    endPos = orderInput;  // only updated not in IDLE state
    setCushion();
  }
  // CCW
  else if (differential < -DEADBAND) {
    motorState = CW;
    endPos = orderInput;  // only updated not in IDLE state
    setCushion();
  }
  // IDLE
  else {
    if (abs(differential) <= PRECISION)  // precision to read target and enter IDLE state
      motorState = IDLE;
    startPos = responseInput;  // only updated in IDLE state
  }
  return;
}

void updateOutputs() {  // updateOutputs() based on state and differential. Most complex.
  if (motorState == IDLE) {
    digitalWrite(EN_PIN, HIGH);  // Disable driver
    stepDelay = baseDelay;
  } else {
    digitalWrite(EN_PIN, LOW);                             // Enable motor driver
    digitalWrite(DIR_PIN, motorState == CW ? HIGH : LOW);  // Apply direction (may be backwards)
    // cushion the speed at the beginning and end of a cycle
    if (startCushion)
      stepDelay -= ACCELERATION;  // decrease period to increase frequency (accelerate)
    else if (endCushion)
      stepDelay += ACCELERATION;  // increase period to decrease frequency (decelerate)
    else
      stepDelay = stepDelay + 0;  // maybe redundant, but this is max-speed, and I want to emphasize it
    // run either Stepper or serial debug
    if (serialMode)
      digitalWrite(EN_PIN, HIGH);  // Disable driver
    else
      runStepper(stepDelay);  // after speed delay of 1 cycle is set, run motor for 1 cycle
  }
  return;
}

void runSerial() {  // to run serial com, disables motor's EN pin
  Serial.print("Order: ");
  Serial.print(orderInput);  // or voltage
  Serial.print("\tResponse: ");
  Serial.print(responseInput);  // or voltage
  Serial.print("\t  Diff: ");
  Serial.print(differential);
  Serial.print("\tState: ");
  switch (motorState) {
    case IDLE: Serial.println("IDLE"); break;
    case CW: Serial.println("CW"); break;
    case CCW: Serial.println("CCW"); break;
  }
  return;
}

// main //////////////////////////////////////////////////////////////////
void setup() {
  pinMode(DIR_PIN, OUTPUT);
  pinMode(PUL_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  if (serialMode) { Serial.begin(9600); }  // optional, for troubleshooting only
}
void loop() {
  updateInputs();
  updateState();
  updateOutputs();
  if (serialMode) { runSerial(); }  // optional, for troubleshooting only
}
