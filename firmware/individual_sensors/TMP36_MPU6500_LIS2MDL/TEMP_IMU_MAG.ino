#include <Wire.h>
#include <math.h>

// --- I2C Addresses ---
#define MUX_ADDR    0x70
#define MPU_ADDR    0x68  // AD0 to GND
#define MAG_ADDR    0x1E  // SDO to 3.3V (as per your previous scan)

// --- Pins ---
#define MUX_RST_PIN 4     // Multiplexer Reset
#define TMP_PIN     14    // Analog Middle Pin of TMP36GZ

// --- Timing ---
unsigned long lastUpdate = 0;
const int interval = 500; // Update every 500ms

// --- Helper: Select Mux Channel ---
void selectBus(uint8_t bus) {
  Wire.beginTransmission(MUX_ADDR);
  Wire.write(1 << bus);
  Wire.endTransmission();
  delay(5); // Stabilization delay
}

// --- Helper: Write to I2C Register ---
void writeReg(uint8_t dev, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(dev);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  // 1. Initialize Pins
  pinMode(MUX_RST_PIN, OUTPUT);
  digitalWrite(MUX_RST_PIN, HIGH);
  analogReadResolution(12); // Higher precision (0-4095) for Teensy 4.1

  // 2. Start I2C
  Wire.begin();
  Wire.setClock(400000); // 400kHz Fast Mode
  delay(500);

  // 3. Init MPU6500 (Channel 1)
  selectBus(1);
  writeReg(MPU_ADDR, 0x6B, 0x00); // Wake up MPU
  
  // 4. Init LIS2MDL (Channel 2)
  selectBus(2);
  writeReg(MAG_ADDR, 0x60, 0x8C); // 100Hz + Continuous Mode

  Serial.println(F("========================================"));
  Serial.println(F("   CUBESAT INTEGRATED SENSOR SYSTEM    "));
  Serial.println(F("========================================\n"));
}

void loop() {
  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();

    // --- READ TMP36 (ANALOG) ---
    float vOut = (analogRead(TMP_PIN) / 4095.0) * 3.3;
    float tempC = (vOut - 0.5) * 100.0;

    // --- READ MPU6500 (CHANNEL 1) ---
    selectBus(1);
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 6);
    float ax = (int16_t)(Wire.read() << 8 | Wire.read()) / 16384.0;
    float ay = (int16_t)(Wire.read() << 8 | Wire.read()) / 16384.0;
    float az = (int16_t)(Wire.read() << 8 | Wire.read()) / 16384.0;

    // --- READ LIS2MDL (CHANNEL 2) ---
    selectBus(2);
    Wire.beginTransmission(MAG_ADDR);
    Wire.write(0x68);
    Wire.endTransmission(false);
    Wire.requestFrom(MAG_ADDR, 6);
    float mx = (int16_t)(Wire.read() | (Wire.read() << 8)) * 0.0015;
    float my = (int16_t)(Wire.read() | (Wire.read() << 8)) * 0.0015;
    float mz = (int16_t)(Wire.read() | (Wire.read() << 8)) * 0.0015;
    float bTotal = sqrt(mx*mx + my*my + mz*mz);

    // --- TELEMETRY OUTPUT ---
    Serial.println(F("----------------------------------------"));
    Serial.printf("TEMP  | %.2f C\n", tempC);
    Serial.printf("IMU   | Accel (g): X:%.2f, Y:%.2f, Z:%.2f\n", ax, ay, az);
    Serial.printf("MAG   | Coord (G): X:%.3f, Y:%.3f, Z:%.3f\n", mx, my, mz);
    Serial.printf("FIELD | Total: %.4f Gauss\n", bTotal);
  }
}