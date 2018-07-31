const int TRIG = PB0;
const int ECHO = PB1;
const int RED = PB2;
const int YELLOW = PB3;
const int GREEN = PB4;

// Max time to spend reading the sensor
const unsigned long READ_MAX_DURATION_MS = 60;
// How often to check for movement in checking mode
const unsigned long CHECK_INTERVAL_MS = 1000; 
// How much movement do we consider movement? Has to be more than this:
const unsigned long THRESHOLD_CM = 5;
// How long before we turn off the lights after not detecting movement?
const unsigned long TIMEOUT_MS = 30000;

const int DISTANCE_GREEN = 60;
const int DISTANCE_YELLOW = 80;
const int DISTANCE_RED=50;

enum Mode {
  Checking,
  Tracking
};

Mode __mode = Checking;

unsigned long __trackingTime = 0;
int __lastColor = -1;

void setup() {
  // Serial.begin(115200);
  
  // Pin modes
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);

  // Turn off all the LEDs
  digitalWrite(RED, 0);
  digitalWrite(YELLOW, 0);
  digitalWrite(GREEN, 0);

  // Default trigger state is low
  digitalWrite(TRIG, 0);
  // Serial.println("Init complete");
  doTracking();
}

// Returns the measured distance in cm, or -1 if nothing could be read.
int measure()
{
  // Serial.println("M1");
  // To measure, we bring TRIG high for 10us, then wait for ECHO to go high and time it.
  // The distance measured is time(us)/58cm or time(us)/148in. The suggested measurement 
  // cycle is over 60ms.
  digitalWrite(TRIG, 1);
  delayMicroseconds(10);
  digitalWrite(TRIG, 0);

  // Now we wait up to 60ms for the echo pulse
  unsigned long startTime = millis();
  while (millis() - startTime < READ_MAX_DURATION_MS && digitalRead(ECHO) == 0);
  // Serial.println("M2");

  // Did we time out?
  if (digitalRead(ECHO) == 0)
  {
    // Serial.println("TO1");
    return -1;
  }

  unsigned long tStart = micros();
  while (millis() - startTime < READ_MAX_DURATION_MS && digitalRead(ECHO) == 1);
  unsigned long duration = micros() - tStart;
  
  // Did we time out?
  if (digitalRead(ECHO) == 1)
  {
    // Serial.println("TO2");
    return -1;
  }

  // Serial.println(duration / 58);
  return duration / 58;
}

unsigned long __checkingTime = 0;
int __lastMeasurement = -1;
void doChecking()
{
  digitalWrite(RED, 0);
  digitalWrite(GREEN, 0);
  digitalWrite(YELLOW, 0);

  if (millis() - __checkingTime > CHECK_INTERVAL_MS)
  {
    
    __checkingTime = millis();
    // We're looking to see if there is any movement.
    int measurement = measure();
    // Serial.print("M: ");
    // Serial.println(measurement);
    if (measurement == -1)
    {
      return;
    }
    
    if (abs(measurement - __lastMeasurement) > THRESHOLD_CM)
    {
      __lastMeasurement = measurement;
      __mode = Tracking;
    }
  } 
}


void doTracking()
{
  bool moving = false;
  int distance = measure();
  if (distance == -1 || abs(distance - __lastMeasurement) > THRESHOLD_CM) {
    moving = true;
    __trackingTime = millis();
  }
  __lastMeasurement = distance;
  
  if (moving) {
    
    digitalWrite(RED, 0);
    digitalWrite(GREEN, 0);
    digitalWrite(YELLOW, 0);

    if (distance > DISTANCE_YELLOW) {
      digitalWrite(YELLOW, 1);
      if (__lastColor != YELLOW) {
        __lastColor = YELLOW;
      }
    } else if (distance > DISTANCE_GREEN) {
      digitalWrite(GREEN, 1);
      if (__lastColor != GREEN) {
        __lastColor = GREEN;
      }
    } else {
      digitalWrite(RED, 1);
      if (__lastColor != RED) {
        __lastColor = RED;
      }
    }
  }

  if (millis() - __trackingTime > TIMEOUT_MS) {
    __lastColor = -1;
    __mode = Checking;
  }
}

void loop() {
  switch(__mode)
  {
    case Checking:
    doChecking();
    break;

    case Tracking:
    // Serial.print("T");
    doTracking();
    break;
  }
}
