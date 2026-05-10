#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  if (!mpu.begin()) {
    Serial.println("Sensor not found!");
    while (1);
  }
  // Setup for clean data
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setFilterBandwidth(MPU6050_FILTER_21_HZ);
  
  // CSV Header
  Serial.println("accX,accY,accZ,gyroX,gyroY,gyroZ");
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Print data for CSV recording
  Serial.print(a.acceleration.x); Serial.print(",");
  Serial.print(a.acceleration.y); Serial.print(",");
  Serial.print(a.acceleration.z); Serial.print(",");
  Serial.print(g.gyro.x);         Serial.print(",");
  Serial.print(g.gyro.y);         Serial.print(",");
  Serial.println(g.gyro.z);

  delay(20); // 50Hz capture rate
}
