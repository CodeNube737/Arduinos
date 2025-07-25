// stepperControl.ino
/* By Mikhail R & AI
  Inputs
    Analog voltage readings from pins A6 and A7, expecting 1-5V, not greater than 5V
    Converted to voltages for diagnostic reference (optional Serial for debugging)

  Control Logic
    differential = order - response
    States: IDLE, CW, CCW based on directional comparison
    Position targets: 240° (CCW), 90° (mid), 300° (CW)

  Output Signals
    DIR pin (D10): HIGH for CW, LOW for CCW
    PUL pin (D8): step signal with speed based on proportional control
*/

#define DIR_PIN 10
#define PUL_PIN 8
#define ORDER_PIN A6 
#define RESPONSE_PIN A7 

int PRECISION = 10;    // How close counts must be to be considered IDLE
int SPEED = 50;        // From 0 (slow) to 100 (fast)
int CUSHION = 30;      // When to start slowing down near target
int PPR = 400;         // Steps per revolution (e.g. 200 for full-step, 400 for half-step)

enum State {IDLE, CW, CCW};
State motorState = IDLE;

void setup() {
  pinMode(DIR_PIN, OUTPUT);
  pinMode(PUL_PIN, OUTPUT);
  analogReference(DEFAULT); // For my Nano: 5V reference
}

void loop() {
  int orderRaw = analogRead(ORDER_PIN);
  int responseRaw = analogRead(RESPONSE_PIN);

  float orderVoltage = orderRaw*5.0 / 1023.0;
  float responseVoltage = responseRaw*5.0 / 1023.0;

  int differential = orderRaw - responseRaw;

  // Determine motor state
  if(abs(differential) <= PRECISION) {
    motorState = IDLE;
  }
  else if (differential >= 0) {
    motorState = CW;
  } else {
    motorState = CCW;
  }

  // Apply direction
  digitalWrite(DIR_PIN, motorState==CW ? HIGH : LOW);

  // step with proportional control and PPR scaling
  if (motorState != IDLE) {
    // Higher differential -> more speed
    int baseDelay = map(abs(differential), PRECISION, 1023, SPEED*2, SPEED); // may need an I
    baseDelay = constrain(baseDelay, 1, 1000); // may need an I

    // Scale delay by PPR: higher PPR = faster stepping
    int stepDelay = baseDelay * 200 / PPR; // 200ppr is the standard for full step/pulse

    digitalWrite(PUL_PIN, HIGH);
    delayMicroseconds(stepDelay); // it seems some libraries leave hi on for a pulse, others for a delay time
    digitalWrite(PUL_PIN, LOW); 
    delayMicroseconds(stepDelay); // but the delay here definitely makes sense
  }
}
