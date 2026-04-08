#include <ESP8266WiFi.h>
#include <Firebase.h>
#include <Servo.h>
#include "secrets.h"

// PIN
#define SERVO_PIN D4
#define TRIG_PIN D5
#define ECHO_PIN D6
#define RED_PIN D1
#define GREEN_PIN D2
#define BLUE_PIN D3
#define BUZZER_PIN D7

Servo gateServo;
Firebase fb(REFERENCE_URL);

bool gateOpen = false;

// ================= ULTRASONIC =================
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

// ================= SERVO SMOOTH =================
void openGate() {
  for (int a = 0; a <= 120; a++) {
    gateServo.write(a);
    delay(20);
  }
}

void closeGate() {
  for (int a = 120; a >= 0; a--) {
    gateServo.write(a);
    delay(20);
  }
}

// ================= BUZZER =================
void beep(int t) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(t);
  digitalWrite(BUZZER_PIN, LOW);
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  gateServo.attach(SERVO_PIN);
  gateServo.write(0);

  // WIFI
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi Connected!");
  Serial.println("Firebase Ready!");
}

void loop() {
  long distance = getDistance();

  Serial.print("Jarak: ");
  Serial.print(distance);
  Serial.println(" cm");

  fb.setInt("smart_parking/distance", distance);

  // ================= MOBIL DATANG =================
  if (distance < 10 && !gateOpen) {
    gateOpen = true;

    Serial.println("GATE OPEN");
    fb.setString("smart_parking/gate", "OPEN");
    fb.setString("smart_parking/rgb", "GREEN");

    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(RED_PIN, LOW);
    digitalWrite(BLUE_PIN, LOW);

    beep(300);
    openGate();
  }

  // ================= MOBIL PERGI → TUTUP =================
  if (distance > 15 && gateOpen) {
    gateOpen = false;

    Serial.println("GATE CLOSE");
    fb.setString("smart_parking/gate", "CLOSE");
    fb.setString("smart_parking/rgb", "RED");

    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(BLUE_PIN, LOW);

    beep(500);
    closeGate();
  }

  // ================= STANDBY =================
  if (distance > 15 && !gateOpen) {
    Serial.println("STANDBY");
    fb.setString("smart_parking/gate", "STANDBY");
    fb.setString("smart_parking/rgb", "BLUE");

    digitalWrite(BLUE_PIN, HIGH);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(RED_PIN, LOW);
  }

  delay(300);
}