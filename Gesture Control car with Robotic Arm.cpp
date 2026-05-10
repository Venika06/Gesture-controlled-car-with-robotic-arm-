#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

// Pins
const int IN1=26, IN2=25, ENA=14, IN3=33, IN4=32, ENB=27; 
const int trigPin=5, echoPin=18;
Servo baseS, shoulderS, elbowS, gripperS;

typedef struct struct_message {
    float x;
    float y;
    bool switchMode;
    int gestureType; // Mistake fixed: matches glove struct
} struct_message;

struct_message incoming;
unsigned long lastPing = 0;
long dist = 999;

void OnDataRecv(const uint8_t * mac, const uint8_t *data, int len) {
  memcpy(&incoming, data, sizeof(incoming));
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  pinMode(trigPin, OUTPUT); pinMode(echoPin, INPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);
  
  baseS.attach(13); shoulderS.attach(12); elbowS.attach(2); gripperS.attach(4);
  if (esp_now_init() == ESP_OK) esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void loop() {
  // 1. Ultrasonic Safety
  if (millis() - lastPing > 60) {
    digitalWrite(trigPin, LOW); delayMicroseconds(2);
    digitalWrite(trigPin, HIGH); delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH, 25000);
    dist = (duration == 0) ? 999 : duration * 0.034 / 2;
    lastPing = millis();
  }

  // 2. Drive Control
  if (incoming.x > 25 && dist > 15) { // Forward
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    analogWrite(ENA, 200); analogWrite(ENB, 200);
  } else if (incoming.x < -25) { // Backward
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
    analogWrite(ENA, 200); analogWrite(ENB, 200);
  } else {
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  }

  // 3. ML Automation Trigger
  if (incoming.gestureType == 1) {
    // Only auto-close gripper when the gesture is detected
    gripperS.write(90); 
    
    // Kept your delay just in case you want to debounce the signal, 
    // but since it's in loop() now and not OnDataRecv, it's safe!
    delay(200); 
  } else {
    // Normal base rotation mode based on tilt
    baseS.write(map(incoming.y, -90, 90, 0, 180)); 
  }
}