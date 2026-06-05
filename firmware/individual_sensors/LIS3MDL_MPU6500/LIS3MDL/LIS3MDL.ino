#include <Wire.h>
#include <math.h>

#define MUX_ADDR 0x70
#define MPU_ADDR 0x68
#define MAG_ADDR 0x1E
#define RST_PIN  4

void select(uint8_t ch) {
  Wire.beginTransmission(MUX_ADDR);
  Wire.write(1 << ch);
  Wire.endTransmission();
  delay(2); 
}

void writeReg(uint8_t dev, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(dev);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, HIGH);
  Wire.begin();
  Wire.setClock(400000);
  
  select(1); writeReg(MPU_ADDR, 0x6B, 0x00); // Wake MPU
  select(2); writeReg(MAG_ADDR, 0x60, 0x8C); // Init MAG (100Hz)
  Serial.println("--- CubeSat Sensors Online ---");
}

void loop() {
  // --- MPU6500 Data ---
  select(1);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  if (Wire.endTransmission() == 0 && Wire.requestFrom(MPU_ADDR, 6) == 6) {
    float ax = (int16_t)(Wire.read() << 8 | Wire.read()) / 16384.0;
    float ay = (int16_t)(Wire.read() << 8 | Wire.read()) / 16384.0;
    float az = (int16_t)(Wire.read() << 8 | Wire.read()) / 16384.0;
    Serial.printf("IMU Coordinate (g) – %.2f, %.2f, %.2f\n", ax, ay, az);
  }

  // --- LIS2MDL Data ---
  select(2);
  Wire.beginTransmission(MAG_ADDR);
  Wire.write(0x68);
  if (Wire.endTransmission() == 0 && Wire.requestFrom(MAG_ADDR, 6) == 6) {
    float mx = (int16_t)(Wire.read() | (Wire.read() << 8)) * 0.0015;
    float my = (int16_t)(Wire.read() | (Wire.read() << 8)) * 0.0015;
    float mz = (int16_t)(Wire.read() | (Wire.read() << 8)) * 0.0015;
    float bTotal = sqrt(mx*mx + my*my + mz*mz);
    
    Serial.printf("MAG Coordinate (G) – %.3f, %.3f, %.3f\n", mx, my, mz);
    Serial.printf("Total Magnetic Field – %.4f Gauss\n", bTotal);
  }

  Serial.println("----------------------------------------");
  delay(500); 
}