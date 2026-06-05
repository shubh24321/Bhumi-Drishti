#include <Wire.h>
#include <MPU6500_WE.h>      // ✅ Correct include for MPU6500
#include <Adafruit_LIS2MDL.h>

// Create sensor objects
MPU6500_WE mpu6500 = MPU6500_WE(0x68);  // I2C address 0x68
Adafruit_LIS2MDL lis2mdl;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Wire.begin();
  Wire.setClock(400000);
  
  // Initialize MPU6500
  if (!mpu6500.init()) {
    Serial.println("MPU6500 does not respond!");
  } else {
    Serial.println("MPU6500 is connected!");
    
    // Auto-calibrate (place sensor flat and don't move)
    Serial.println("Place MPU6500 flat - calibrating in 2 seconds...");
    delay(2000);
    mpu6500.autoOffsets();
    Serial.println("Calibration done!");
    
    // Set ranges
    mpu6500.setAccRange(MPU6500_ACC_RANGE_2G);
    mpu6500.setGyroRange(MPU6500_GYRO_RANGE_250);
    mpu6500.enableAccDLPF(true);
    mpu6500.setAccDLPF(MPU6500_DLPF_6);
  }
  
  // Initialize LIS2MDL
  if (!lis2mdl.begin()) {
    Serial.println("LIS2MDL not detected!");
  } else {
    Serial.println("LIS2MDL connected!");
  }
  
  delay(100);
}

void loop() {
  // Read MPU6500 data
  xyzFloat accRaw = mpu6500.getAccRawValues();
  xyzFloat gyrRaw = mpu6500.getGyrRawValues();
  float temperature = mpu6500.getTemperature();
  
  // Read LIS2MDL magnetometer
  sensors_event_t mag_event;
  lis2mdl.getEvent(&mag_event);
  
  // Print data: Ax,Ay,Az,Gx,Gy,Gz,Temp,Mx,My,Mz
  Serial.print(accRaw.x); Serial.print(",");
  Serial.print(accRaw.y); Serial.print(",");
  Serial.print(accRaw.z); Serial.print(",");
  Serial.print(gyrRaw.x); Serial.print(",");
  Serial.print(gyrRaw.y); Serial.print(",");
  Serial.print(gyrRaw.z); Serial.print(",");
  Serial.print(temperature); Serial.print(",");
  Serial.print(mag_event.magnetic.x); Serial.print(",");
  Serial.print(mag_event.magnetic.y); Serial.print(",");
  Serial.println(mag_event.magnetic.z);
  
  delay(10);  // 100Hz update rate
}