// stepperControl.ino
/* By Mikhail R, 2025/aug/1
 *    Runs a stepper motor through a typical driver with DIR, PUL, and EN inputs
 * Revised same day, after some short debugging and clarifying comments
*/
////////////////////////////////////////////////////////////////////////

// pins
#define PUL_PIN 8
#define DIR_PIN 10
#define EN_PIN 12
#define ORDER_PIN A6
#define RESPONSE_PIN A7
// numeric constants for fine-tuning
#define PPR 400         // created because Leadshine ctrllr sets ppr. 200 = full, 400 = 1/2... etc. Adjust MIN_PULSE with PPR.
#define MIN_PULSE 1250  // in µs, eg. 1250us = a 2.5ms period, or 400Hz base-speed, at 400 ppr, that's 1rev/sec. Adjust MIN_PULSE with PPR.
#define SPEED 90        // percentage of RDNY-speed
#define ACCELERATION 1  // for CUSHION, must be positive. e.g. 5*30=150µs gained from acceleration
#define PRECISION 2     // as small an integer as possible but can still read feedback (response)
#define DEADBAND 16     // should be at least enough to cover noise, for exiting IDLE. must be larger at smaller PPR.
#define CUSHION 30      // as big as desired, speed-up/slow-down before leaving/entering IDLE
#define RDNY 10         // seconds per revolution (anticipated speed)
// global variables
enum State { IDLE,
             CW,
             CCW };
State motorState = IDLE;
bool startCushion = false;                                         // a sort-of sub state flag
bool endCushion = false;                                           // a sort-of sub state flag
int baseDelay = (MIN_PULSE * 2 * 200 / PPR * 100 / SPEED * RDNY);  // see notes
int stepDelay = baseDelay;                                         // starting value
int orderInput, responseInput, differential, startPos, endPos;     // should never exceed ±1023, or ±5V
const int MINinput = 204;                                          // 1 Volt (4mA)... currently unused
const int MAXinput = 1023;                                         // 5 Volts (20mA)
//int counter = 0; // may help for future iterations?
// helper functions

//int constrainInput(int value) { return constrain(value, MINinput, MAXinput); } //.. currently unused

void runStepper(int stepDelay) {
  if (stepDelay <= MIN_PULSE)  // speed limit on PUL_PIN. The lower the period, the higher the freq.
    stepDelay = baseDelay;     // MIN_PULSE should be 1 rev/second
  digitalWrite(PUL_PIN, HIGH);
  delayMicroseconds(stepDelay);  // this is the briefer of the two
  digitalWrite(PUL_PIN, LOW);
  delayMicroseconds(stepDelay);
  return;
}

void setCushion() {  // set flags (sub-state)
  startCushion = abs(responseInput - startPos) <= CUSHION;
  endCushion = abs(responseInput - endPos) <= CUSHION;
  if (motorState == IDLE) { // IDLE state should turn-off all cushion/accel flags
    startCushion = false;
    endCushion = false;
  }
  return;
}

void updateInputs() {  // updateInputs() and subtract to get int differential
  orderInput = analogRead(ORDER_PIN);
  responseInput = analogRead(RESPONSE_PIN);
  differential = orderInput - responseInput;
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
  if (differential > DEADBAND) {  // buffer to exit IDLE state
    motorState = CW;
    setCushion();
  }
  // CCW
  else if (differential < -DEADBAND) {
    motorState = CCW;
    setCushion();
  }
  // IDLE
  else {
    if (abs(differential) <= PRECISION)  // precision to read target and trigger IDLE despite over-stepping and noise
      motorState = IDLE;
    startPos = responseInput;  // only updated in IDLE state
    endPos = orderInput;       // only updated in IDLE state
  }
  return;
}

void updateOutputs() {  // updateOutputs() based on state and differential. Most complex.

  digitalWrite(DIR_PIN, motorState == CW ? HIGH : LOW);  // Apply direction (may be reversed)

  if (motorState == IDLE) {
    digitalWrite(EN_PIN, HIGH);  // Disable driver
    stepDelay = baseDelay;
  } else {
    digitalWrite(EN_PIN, LOW);  // Enable driver
    // set speed delay of a cycle
    if (startCushion)
      stepDelay -= ACCELERATION;  // decrease period to increase frequency
    else if (endCushion)
      stepDelay += ACCELERATION;  // increase period to decrease frequency
    else
      stepDelay = stepDelay + 0;  // maybe redundant, but this is max-speed, and I want to emphasize it
    runStepper(stepDelay);        // after speed delay of 1 cycle is set, run motor for 1 cycle
  }
  return;
}

void runSerial() {  // to run serial com
  Serial.print("Order: ");
  Serial.print(orderInput);  // or voltage
  Serial.print("V\tResponse: ");
  Serial.print(responseInput);  // or voltage
  Serial.print("V\t  Diff: ");
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
  //Serial.begin(9600); // optional, for troubleshooting only
}
void loop() {
  updateInputs();
  updateState();
  updateOutputs();
  //runSerial(); // optional, for troubleshooting only
}
