#include <Glove_Control_inferencing.h> // Ensure this matches your ZIP name
#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

uint8_t broadcastAddress[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}; // Your Car's MAC

typedef struct struct_message {
    float x;
    float y;
    bool switchMode;
    int gestureType; // Mistake fixed: using gestureType instead of potValue
} struct_message;

struct_message myData;
Adafruit_MPU6050 mpu;
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (!mpu.begin()) while (1);

  if (esp_now_init() != ESP_OK) return;
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  esp_now_add_peer(&peerInfo);
}

void loop() {
  sensors_event_t a, g, temp;
  
  // 1. Fill Buffer for ML
  for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i += 6) {
    mpu.getEvent(&a, &g, &temp);
    features[i + 0] = a.acceleration.x;
    features[i + 1] = a.acceleration.y;
    features[i + 2] = a.acceleration.z;
    features[i + 3] = g.gyro.x;
    features[i + 4] = g.gyro.y;
    features[i + 5] = g.gyro.z;
    delay(10); 
  }

  // 2. Run ML Inference
  signal_t signal;
  numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  ei_impulse_result_t result = { 0 };
  run_classifier(&signal, &result, false);

  // 3. Assign Data
  myData.x = map(a.acceleration.x, -10, 10, -90, 90);
  myData.y = map(a.acceleration.y, -10, 10, -90, 90);
  myData.switchMode = (digitalRead(15) == LOW);
  
  // Logic: If gesture '1' (e.g. Pick Up) is > 80% sure
  myData.gestureType = (result.classification[1].value > 0.8) ? 1 : 0;

  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
}
