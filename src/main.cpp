#include <Arduino.h>

#define LED 2
#define HALL_SENSOR 14

const float circuitLength = 0.87962; // Length of the wheel in m

bool prevHallSensorValue;
unsigned long lastDetectionTime = 0;

void blink();

void setup()
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(HALL_SENSOR, INPUT);

  prevHallSensorValue = digitalRead(HALL_SENSOR);

  for (int i = 0; i < 3; i++)
  {
    blink();
  }

  Serial.println("Setup complete");
}

void loop()
{
  int hallSensorValue = digitalRead(HALL_SENSOR);

  if (prevHallSensorValue && hallSensorValue == LOW)
  {
    digitalWrite(LED, LOW);
    prevHallSensorValue = hallSensorValue;
    // Serial.print(".");
    unsigned long currentTime = millis();
    float speedMs = circuitLength / (currentTime - lastDetectionTime) * 1000; // Convert to m/s
    float speedKmh = speedMs * 3.6; // Convert to km/h
    Serial.print("Speed: ");
    Serial.print(speedMs);
    Serial.print(" m/s");
    Serial.print(" | [");
    Serial.print(speedKmh);
    Serial.println(" km/h]");
    lastDetectionTime = currentTime;
  }
  else
  {
    digitalWrite(LED, HIGH);
    prevHallSensorValue = hallSensorValue;
  }
}

void blink()
{
  digitalWrite(LED, LOW);
  delay(500);
  digitalWrite(LED, HIGH);
  delay(500);
}
