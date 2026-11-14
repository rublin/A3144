#include <Arduino.h>

#include "common/mavlink.h"

#define LED 4
#define HALL_SENSOR 14

const float circuitLength = 0.87962;  // Length of the wheel in m
const uint8_t MY_SYSID = 1;           // same as FC's SYSID_THISMAV
const uint8_t MY_COMPID = MAV_COMP_ID_ONBOARD_COMPUTER;

bool prevHallSensorValue;
uint32_t lastDetectionTime = 0;
uint32_t lastSendTime = 0;
uint32_t lastHeartbeatTime = 0;

float speedMs = 0;
float speedKmh = 0;
float odoM = 0;

void sendHeartbeat();
void sendNamedFloat(const char* name, float value);

void setup() {
  Serial.begin(115200);
  Serial1.begin(57600);  // to FC (TX only, GPIO2)
  pinMode(LED, OUTPUT);
  pinMode(HALL_SENSOR, INPUT);

  prevHallSensorValue = digitalRead(HALL_SENSOR);

  Serial.println("\n\nSetup complete");
}

void loop() {
  int hallSensorValue = digitalRead(HALL_SENSOR);
  uint32_t now = millis();

  if (now - lastSendTime >= 100) {  // 10 Hz
    lastSendTime = now;
    sendNamedFloat("SPEED_MS", speedMs);
    sendNamedFloat("SPEED_KMH", speedKmh);
    sendNamedFloat("ODO_M", odoM);
  }

  if (now - lastHeartbeatTime >= 1000) {  // 1 Hz
    lastHeartbeatTime = now;
    sendHeartbeat();
  }

  if (prevHallSensorValue && hallSensorValue == LOW) {
    digitalWrite(LED, HIGH);
    prevHallSensorValue = hallSensorValue;
    odoM += circuitLength;
    speedMs = circuitLength / (now - lastDetectionTime) * 1000;
    speedKmh = speedMs * 3.6;  // Convert to km/h
    Serial.print("Speed: ");
    Serial.print(speedMs);
    Serial.print(" m/s");
    Serial.print(" | [");
    Serial.print(speedKmh);
    Serial.println(" km/h]");
    lastDetectionTime = now;
    return;
  }

  if (now - lastDetectionTime > 5000) {
    speedMs = 0;
    speedKmh = 0;
  }

  digitalWrite(LED, LOW);
  prevHallSensorValue = hallSensorValue;
}

void sendHeartbeat() {
  mavlink_message_t msg;
  uint8_t buf[MAVLINK_MAX_PACKET_LEN];
  mavlink_msg_heartbeat_pack(MY_SYSID, MY_COMPID, &msg,
                             MAV_TYPE_ONBOARD_CONTROLLER, MAV_AUTOPILOT_INVALID,
                             0, 0, MAV_STATE_ACTIVE);
  Serial1.write(buf, mavlink_msg_to_send_buffer(buf, &msg));
}

void sendNamedFloat(const char* name, float value) {
  char n[10] = {0};  // field is exactly 10 chars, not null-terminated
  strncpy(n, name, 10);
  mavlink_message_t msg;
  uint8_t buf[MAVLINK_MAX_PACKET_LEN];
  mavlink_msg_named_value_float_pack(MY_SYSID, MY_COMPID, &msg, millis(), n,
                                     value);
  Serial1.write(buf, mavlink_msg_to_send_buffer(buf, &msg));
}
