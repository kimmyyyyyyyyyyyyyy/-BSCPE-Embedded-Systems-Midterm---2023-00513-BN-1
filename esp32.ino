#include <ESP32Servo.h>  // Include Servo library for ESP32

// ESP32 Pin setup
const int ldrPin = 34;         // ADC-capable pin for LDR
const int ledPin = 13;         // GPIO for LED
const int trigPin = 18;        // GPIO for Ultrasonic Trigger
const int echoPin = 19;        // GPIO for Ultrasonic Echo
const int servoPin = 5;        // PWM-capable GPIO for Servo

// Lumen and Distance thresholds
const float lumenThreshold = 20.0;
const float distanceThreshold = 20.0; // cm

Servo myServo;  // Create servo object

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.begin(115200);

  myServo.attach(servoPin);     // Attach servo to GPIO pin
  myServo.write(90);            // Start at neutral position
}

void loop() {
  // 1. Read LDR value (ESP32 ADC range is 0–4095)
  int analogValue = analogRead(ldrPin);
  float voltage = analogValue * (3.3 / 4095.0);  // ESP32 is 3.3V logic

  // Estimate resistance of LDR (using 10k pull-down resistor)
  float resistance = (3.3 - voltage) * 10000.0 / voltage;

  // Estimate lux (rough formula)
  float lux = 500 / (resistance / 1000.0);

  // 2. Measure distance
  float measuredDistance = measureDistanceCM();

  // 3. Apply calibration
  float correctedDistance = 0.95 * measuredDistance - 0.5;

  // 4. Debug output
  Serial.print("Lux: ");
  Serial.print(lux);
  Serial.print(" | Corrected Distance: ");
  Serial.print(correctedDistance);
  Serial.println(" cm");

  // 5. LED control logic
  if (lux < lumenThreshold && correctedDistance <= distanceThreshold) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }

  // 6. Servo motor control logic (map distance to angle)
  int angle = map(correctedDistance, 5, 30, 0, 180);  // Map 5–30 cm to 0–180°
  angle = constrain(angle, 0, 180);                  // Clamp within valid range
  myServo.write(angle);                              // Set servo position

  delay(1000);
}

// Function to measure distance using HC-SR04 on ESP32
float measureDistanceCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // 30 ms timeout
  float distance = duration * 0.034 / 2.0;
  return distance;
}  