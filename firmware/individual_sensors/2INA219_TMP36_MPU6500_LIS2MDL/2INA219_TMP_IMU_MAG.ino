#include <Wire.h>
#include <Adafruit_INA219.h>
#include <math.h>

// --- I2C Addresses ---
#define MUX_ADDR    0x70
#define MPU_ADDR    0x68
#define MAG_ADDR    0x1E
#define INA_ADDR    0x40  // Both INAs use 0x40 since they are on different Mux channels

// --- Pins ---
#define MUX_RST_PIN 4     
#define TMP_PIN     14    

// Create two INA219 objects
Adafruit_INA219 ina_1; // On Channel 3
Adafruit_INA219 ina_2; // On Channel 4

unsigned long lastUpdate = 0;
const int interval = 1000; 

void selectBus(uint8_t bus) {
  Wire.beginTransmission(MUX_ADDR);
  Wire.write(1 << bus);
  Wire.endTransmission();
  delay(5); 
}

void writeReg(uint8_t dev, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(dev);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  pinMode(MUX_RST_PIN, OUTPUT);
  digitalWrite(MUX_RST_PIN, HIGH);
  analogReadResolution(12);

  Wire.begin();
  Wire.setClock(400000);
  delay(500);

  // Init MPU6500 (Ch 1)
  selectBus(1);
  writeReg(MPU_ADDR, 0x6B, 0x00);
  
  // Init LIS2MDL (Ch 2)
  selectBus(2);
  writeReg(MAG_ADDR, 0x60, 0x8C);

  // Init INA219 #1 (Ch 3)
  selectBus(3);
  if (ina_1.begin()) Serial.println(">> INA_1 Initialized (Ch 3)");

  // Init INA219 #2 (Ch 4)
  selectBus(4);
  if (ina_2.begin()) Serial.println(">> INA_2 Initialized (Ch 4)");

  Serial.println(F("\n--- CUBESAT SENSOR ARRAY ONLINE ---\n"));
}

void loop() {
  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();

    // 1. TEMP (Analog)
    float vTemp = (analogRead(TMP_PIN) / 4095.0) * 3.3;
    float tempC = (vTemp - 0.5) * 100.0;

    // 2. IMU (Ch 1)
    selectBus(1);
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 6);
    float ax = (int16_t)(Wire.read() << 8 | Wire.read()) / 16384.0;
    float ay = (int16_t)(Wire.read() << 8 | Wire.read()) / 16384.0;
    float az = (int16_t)(Wire.read() << 8 | Wire.read()) / 16384.0;

    // 3. MAG (Ch 2)
    selectBus(2);
    Wire.beginTransmission(MAG_ADDR);
    Wire.write(0x68);
    Wire.endTransmission(false);
    Wire.requestFrom(MAG_ADDR, 6);
    float mx = (int16_t)(Wire.read() | (Wire.read() << 8)) * 0.0015;
    float my = (int16_t)(Wire.read() | (Wire.read() << 8)) * 0.0015;
    float mz = (int16_t)(Wire.read() | (Wire.read() << 8)) * 0.0015;
    float bTotal = sqrt(mx*mx + my*my + mz*mz);

    // 4. VOLTAGE SENSORS (Ch 3 & 4)
    selectBus(3);
    float busV1 = ina_1.getBusVoltage_V();
    
    selectBus(4);
    float busV2 = ina_2.getBusVoltage_V();

    // --- CLEAN TELEMETRY OUTPUT ---
    Serial.println(F("═════════════ TELEMETRY ═════════════"));
    Serial.printf("THERMAL | Temp: %.2f C\n", tempC);
    Serial.printf("VOLTS   | Rail_1: %.2f V | Rail_2: %.2f V\n", busV1, busV2);
    Serial.printf("IMU     | Accel (g): X:%.2f, Y:%.2f, Z:%.2f\n", ax, ay, az);
    Serial.printf("MAG     | Field: %.4f Gauss\n", bTotal);
    Serial.println(F("═════════════════════════════════════\n"));
  }
}