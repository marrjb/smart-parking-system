#include <ESP8266WiFi.h>        // Library untuk koneksi WiFi ESP8266
#include <Firebase.h>           // Library untuk koneksi ke FIrebase
#include <Servo.h>              // Library untuk mengontrol servo
#include "secrets.h"            // File berisi SSID, password, dan URL Firebase

// PIN
#define SERVO_PIN D4            // Pin servo
#define TRIG_PIN D5             // Pin trigger ultrasonic
#define ECHO_PIN D6             // Pin echo ultrasonic
#define RED_PIN D1              // Pin LED merah
#define GREEN_PIN D2            // Pin LED hijau
#define BLUE_PIN D3             // Pin LED biru
#define BUZZER_PIN D7           // Pin buzzer

Servo gateServo;                // Membuat objek servo
Firebase fb(REFERENCE_URL);     // Inisialisasi Firebase dengan URL

bool gateOpen = false;          // Status gerbang (false = tertutup)

// ================= ULTRASONIC =================
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);      // Matikan trigger dulu
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);     // Kirim sinyal ultrasonic
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);   // Baca waktu pantulan (timeout 30ms)
  if (duration == 0) return 999;    // Kalau gagal baca, anggap jarak jauh
  return duration * 0.034 / 2;      // Konversi waktu jadi jarak (cm)
}

// ================= SERVO =================
void openGate() {
  for (int a = 0; a <= 120; a++) {    // Putar servo dari 0 ke 120 derajat
    gateServo.write(a);               // Set posisi servo
    delay(20);                        // Delay biar gerakan halus
  }
}

void closeGate() {
  for (int a = 120; a >= 0; a--) {    // Putar servo dari 120 ke 0 derajat
    gateServo.write(a);
    delay(20);
  }
}

// ================= BUZZER =================
void beep(int t) {
  digitalWrite(BUZZER_PIN, HIGH);     // Nyalakan buzzer
  delay(t);                           // Tahan sesuai durasi
  digitalWrite(BUZZER_PIN, LOW);      // Matikan buzzer
}

void setup() {
  Serial.begin(115200);               // Mulai komunikasi serial

  pinMode(TRIG_PIN, OUTPUT);          // Set pin trigger sebagai output
  pinMode(ECHO_PIN, INPUT);           // Set pin echo sebagai input
  pinMode(RED_PIN, OUTPUT);           // LED merah output
  pinMode(GREEN_PIN, OUTPUT);         // LED merah output
  pinMode(BLUE_PIN, OUTPUT);          // LED biru output
  pinMode(BUZZER_PIN, OUTPUT);        // Buzzer output

  gateServo.attach(SERVO_PIN);        // Hubungkan servo ke pin
  gateServo.write(0);                 // Hubungkan servo ke pin

  // WIFI
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);       // Mulai koneksi WiFi
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {     // Tunggu sampai terkoneksi
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi Connected!");        // WiFi berhasil tersambung
  Serial.println("Firebase Ready!");          // Firebase siap digunakan
}

void loop() {
  long distance = getDistance();                    // Ambil jarak dari sensor
  Serial.print("Jarak: ");
  Serial.print(distance);
  Serial.println(" cm");
  fb.setInt("smart_parking/distance", distance);    // Kirim data ke Firebase

  // ================= SERVO BUKA =================
  if (distance < 10 && !gateOpen) {                 // Jika ada objek dekat dan gerbang masih tertutup
    gateOpen = true;                                // Ubah status jadi terbuka
    Serial.println("GATE OPEN");
    fb.setString("smart_parking/gate", "OPEN");     // Update status ke Firebase
    fb.setString("smart_parking/rgb", "GREEN");     // Update status ke Firebase
    digitalWrite(GREEN_PIN, HIGH);                  // Nyalakan LED hijau
    digitalWrite(RED_PIN, LOW);
    digitalWrite(BLUE_PIN, LOW);
    beep(300);                                      // Bunyi buzzer sebentar
    openGate();                                     // Buka gerbang
  }

  // ================= SERVO TUTUP =================
  if (distance > 15 && gateOpen) {                  // Jika objek sudah jauh dan gerbang terbuka
    gateOpen = false;                               // Ubah status jadi tertutup
    Serial.println("GATE CLOSE");
    fb.setString("smart_parking/gate", "CLOSE");    // Update Firebase
    fb.setString("smart_parking/rgb", "RED");       // Warna merah
    digitalWrite(RED_PIN, HIGH);                    // Nyalakan LED merah
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(BLUE_PIN, LOW);
    beep(500);                                      // Bunyi buzzer lebih lama
    closeGate();                                    // Tutup gerbang
  }

  // ================= STANDBY =================
  if (distance > 15 && !gateOpen) {                 // Jika tidak ada objek dan gerbang tertutup
    Serial.println("STANDBY");
    fb.setString("smart_parking/gate", "STANDBY");  // Status standby
    fb.setString("smart_parking/rgb", "BLUE");      // Warna biru
    digitalWrite(BLUE_PIN, HIGH);                   // Nyalakan LED biru
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(RED_PIN, LOW);
  }

  delay(300);           
}