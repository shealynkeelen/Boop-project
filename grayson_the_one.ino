#include <Ultrasonic.h>

// ---------------- MOTOR SHIELD PINS ----------------
const int PWM2A = 11;
const int PWM2B = 3;
const int PWM0A = 6;
const int PWM0B = 5;
const int DIR_CLK = 4;
const int DIR_EN = 7;
const int DATA = 8;
const int DIR_LATCH = 12;

// ---------------- MOTOR DIRECTIONS ----------------
const int Move_Forward  = 39;
const int Right_Rotate  = 149;
const int Left_Rotate   = 106;
const int Stop          = 0;

// ---------------- MOTOR SPEEDS ----------------
int Speed1 = 110;
int Speed2 = 110;
int Speed3 = 110;
int Speed4 = 110;

// ---------------- ULTRASONIC SENSOR ----------------
Ultrasonic ultrasonic(A0, A1);   // trig = A0, echo = A1

// ---------------- LINE SENSOR ----------------
const int lineSensorPin = A2;

// ---------------- TUNING VALUES ----------------
const unsigned long sweepTime = 3000;
const unsigned long sampleDelay = 70;
const int minValidDistance = 2;
const int maxValidDistance = 200;
const int stopDistance = 15;
const unsigned long forwardCheckDelay = 10;

// ---------------- MOTOR FUNCTION ----------------
void Motor(int Dir, int Speed1, int Speed2, int Speed3, int Speed4) {
  analogWrite(PWM2A, Speed1);
  analogWrite(PWM2B, Speed2);
  analogWrite(PWM0A, Speed3);
  analogWrite(PWM0B, Speed4);

  digitalWrite(DIR_LATCH, LOW);
  shiftOut(DATA, DIR_CLK, MSBFIRST, Dir);
  digitalWrite(DIR_LATCH, HIGH);
}

// ---------------- HELPERS ----------------
int getDistanceCM() {
  int d = ultrasonic.read();

  if (d < minValidDistance || d > maxValidDistance) {
    return -1;
  }
  return d;
}

bool blackLineDetected() {
  return digitalRead(lineSensorPin) == HIGH;
}

void stopCar() {
  Motor(Stop, 0, 0, 0, 0);
}

void rotateRight() {
  Motor(Right_Rotate, Speed1, Speed2, Speed3, Speed4);
}

void rotateLeft() {
  Motor(Left_Rotate, Speed1, Speed2, Speed3, Speed4);
}

void moveForward() {
  Motor(Move_Forward, Speed1, Speed2, Speed3, Speed4);
}

void turnToClosestObject(unsigned long bestTime) {
  unsigned long rightTurnTime = bestTime;
  unsigned long leftTurnTime = sweepTime - bestTime;

  const unsigned long rightCorrection = 150;
  const unsigned long leftCorrection = 150;

  Serial.print("Right turn time: ");
  Serial.println(rightTurnTime);

  Serial.print("Left turn time: ");
  Serial.println(leftTurnTime);

  if (rightTurnTime <= leftTurnTime) {
    Serial.println("Turning RIGHT to closest object...");
    rotateRight();
    delay(rightTurnTime + rightCorrection);
  } else {
    Serial.println("Turning LEFT to closest object...");
    rotateLeft();
    delay(leftTurnTime + leftCorrection);
  }

  stopCar();
  delay(500);
}

void setup() {
  pinMode(DIR_CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(DIR_EN, OUTPUT);
  pinMode(DIR_LATCH, OUTPUT);
  pinMode(PWM0B, OUTPUT);
  pinMode(PWM0A, OUTPUT);
  pinMode(PWM2A, OUTPUT);
  pinMode(PWM2B, OUTPUT);

  pinMode(lineSensorPin, INPUT);

  Serial.begin(9600);

  digitalWrite(DIR_EN, LOW);

  stopCar();
  delay(1000);
}

void loop() {
  int closestDistance = 9999;
  unsigned long bestTime = 0;

  moveForward();
  delay(2000);
  stopCar();
  delay(300);

  unsigned long startTime = millis();
  Serial.println("Starting 360 sweep...");
  rotateRight();

  while (millis() - startTime < sweepTime) {
    if (blackLineDetected()) {
      Serial.println("Black line detected during sweep. Stopping.");
      stopCar();
      while (true) { }
    }

    int distance = getDistanceCM();

    if (distance != -1) {
      Serial.print("Sweep distance: ");
      Serial.print(distance);
      Serial.println(" cm");

      if (distance < closestDistance) {
        closestDistance = distance;
        bestTime = millis() - startTime;
      }
    }

    delay(sampleDelay);
  }

  stopCar();
  delay(500);

  if (closestDistance == 9999) {
    Serial.println("No valid object found.");
    delay(1000);
    return;
  }

  Serial.print("Closest distance found: ");
  Serial.println(closestDistance);

  Serial.print("Best time: ");
  Serial.println(bestTime);

  turnToClosestObject(bestTime);

  Serial.println("Moving toward object...");

  while (true) {
    if (blackLineDetected()) {
      Serial.println("Black line detected. Stopping before crossing.");
      stopCar();
      while (true) { }
    }

    int distance = getDistanceCM();

    Serial.print("Live distance: ");
    Serial.print(distance);
    Serial.println(" cm");


    moveForward();
    delay(forwardCheckDelay);
  }
}