/*
 * LIS2MDL Simple Test Code
 * Tests basic functionality of LIS2MDL magnetometer on PCA9548A Channel 2
 * Teensy 4.1 - Pin 18 (SDA), Pin 19 (SCL)
 * 
 * Wiring:
 *   PCA9548A Channel 2: SC2 (SCL) and SD2 (SDA) connected to LIS2MDL
 *   LIS2MDL SDO to GND (I2C address = 0x1E)
 */

#include <Wire.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

#define MUX_ADDR        0x70    // PCA9548A address (A0, A1, A2 to GND)
#define MUX_CHANNEL     2       // Channel 2 (SC2/SD2) for LIS2MDL
#define LIS2MDL_ADDR    0x1E    // LIS2MDL address (SDO to GND)
#define RESET_PIN       4       // PCA9548A RST pin

// LIS2MDL Registers
#define LIS2MDL_WHO_AM_I        0x4F
#define LIS2MDL_CTRL_REG1       0x60
#define LIS2MDL_CTRL_REG2       0x61
#define LIS2MDL_CTRL_REG3       0x62
#define LIS2MDL_STATUS_REG      0x67
#define LIS2MDL_OUT_X_L         0x68
#define LIS2MDL_OUT_X_H         0x69
#define LIS2MDL_OUT_Y_L         0x6A
#define LIS2MDL_OUT_Y_H         0x6B
#define LIS2MDL_OUT_Z_L         0x6C
#define LIS2MDL_OUT_Z_H         0x6D

// Expected WHO_AM_I value
#define LIS2MDL_EXPECTED_ID     0x40

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

unsigned long lastCheck = 0;
unsigned long lastData = 0;
int16_t magX = 0, magY = 0, magZ = 0;
bool lis2mdl_found = false;

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  Serial.begin(115200);
  
  // Wait for Serial Monitor
  while (!Serial && millis() < 3000);
  
  delay(500);
  
  Serial.println("\n========================================");
  Serial.println("LIS2MDL Simple Test on PCA9548A");
  Serial.println("Teensy 4.1");
  Serial.println("========================================\n");
  
  // Initialize Reset Pin
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);
  delay(100);
  
  // Initialize I2C
  Wire.begin();
  Wire.setClock(400000);  // 400 kHz
  
  Serial.println("System initialized. Testing devices...\n");
  
  // Test PCA9548A
  testPCA9548A();
  delay(500);
  
  // Test LIS2MDL
  testLIS2MDL();
  delay(500);
  
  // Initialize LIS2MDL if found
  if (lis2mdl_found) {
    initLIS2MDL();
    Serial.println("\nStarting continuous data read...");
    Serial.println("========================================\n");
  }
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  unsigned long now = millis();
  
  // Read data every 100ms
  if (now - lastData >= 100) {
    lastData = now;
    
    if (lis2mdl_found) {
      readLIS2MDL();
      printMagData();
    }
  }
  
  // Check connection every 5 seconds
  if (now - lastCheck >= 5000) {
    lastCheck = now;
    
    Serial.println("\n--- Periodic Connection Check ---");
    testPCA9548A();
    if (lis2mdl_found) {
      testLIS2MDL();
    }
    Serial.println("-----------------------------------\n");
  }
}

// ============================================================================
// TEST & INITIALIZATION FUNCTIONS
// ============================================================================

void testPCA9548A() {
  Serial.print("Testing PCA9548A at 0x70... ");
  
  Wire.beginTransmission(MUX_ADDR);
  byte error = Wire.endTransmission();
  
  if (error == 0) {
    Serial.println("✓ FOUND");
    return;
  }
  else if (error == 4) {
    Serial.println("✗ BUS ERROR - Check pull-up resistors");
  }
  else {
    Serial.print("✗ NOT FOUND (Error: ");
    Serial.print(error);
    Serial.println(")");
  }
}

void testLIS2MDL() {
  Serial.print("Testing LIS2MDL at 0x1E on Channel ");
  Serial.print(MUX_CHANNEL);
  Serial.print("... ");
  
  // Select channel
  selectMuxChannel(MUX_CHANNEL);
  delay(10);
  
  // Read WHO_AM_I
  uint8_t whoAmI = readRegister(LIS2MDL_ADDR, LIS2MDL_WHO_AM_I);
  
  Serial.print("WHO_AM_I = 0x");
  Serial.print(whoAmI, HEX);
  Serial.print(" (Expected: 0x");
  Serial.print(LIS2MDL_EXPECTED_ID, HEX);
  Serial.print(") ");
  
  if (whoAmI == LIS2MDL_EXPECTED_ID) {
    Serial.println("✓ FOUND");
    lis2mdl_found = true;
  }
  else {
    Serial.println("✗ NOT FOUND or WRONG ID");
    lis2mdl_found = false;
  }
}

void initLIS2MDL() {
  Serial.println("\nInitializing LIS2MDL...");
  
  selectMuxChannel(MUX_CHANNEL);
  delay(10);
  
  // CTRL_REG1: Enable continuous mode, ODR = 10 Hz
  // Bit 7: COMP_TEMP_EN = 1 (Temperature compensation enabled)
  // Bit 5-4: ODR[1:0] = 00 (10 Hz)
  // Bit 1: RESET_ODR = 0
  // Bit 0: ST = 0 (Self-test disabled)
  writeRegister(LIS2MDL_ADDR, LIS2MDL_CTRL_REG1, 0x80);
  
  // CTRL_REG2: INT_on_PIN disabled, RESET_on_INT disabled
  writeRegister(LIS2MDL_ADDR, LIS2MDL_CTRL_REG2, 0x00);
  
  // CTRL_REG3: Normal operation mode
  // Bit 6: OFFSET_TEMP_EN = 1 (Temperature offset enabled)
  // Bit 4: SELF_TEST = 0
  // Bit 1: LP = 0 (Normal power)
  // Bit 0: MD[1:0] = 00 (Continuous mode)
  writeRegister(LIS2MDL_ADDR, LIS2MDL_CTRL_REG3, 0x40);
  
  delay(100);
  
  Serial.println("LIS2MDL configured:");
  Serial.println("  - Continuous conversion mode");
  Serial.println("  - 10 Hz output data rate");
  Serial.println("  - Temperature compensation enabled");
  Serial.println("  - Ready for data reading");
}

// ============================================================================
// DATA READING FUNCTION
// ============================================================================

void readLIS2MDL() {
  selectMuxChannel(MUX_CHANNEL);
  delay(5);
  
  // Check if data is ready
  uint8_t status = readRegister(LIS2MDL_ADDR, LIS2MDL_STATUS_REG);
  
  // Bit 0: DRDY (Data Ready)
  if ((status & 0x01) == 0) {
    return;  // Data not ready yet
  }
  
  // Read 6 bytes: X_L, X_H, Y_L, Y_H, Z_L, Z_H
  Wire.beginTransmission(LIS2MDL_ADDR);
  Wire.write(LIS2MDL_OUT_X_L | 0x80);  // Set MSB for auto-increment
  Wire.endTransmission();
  
  if (Wire.requestFrom(LIS2MDL_ADDR, 6) != 6) {
    Serial.println("ERROR: Failed to read 6 bytes");
    return;
  }
  
  uint8_t xL = Wire.read();
  uint8_t xH = Wire.read();
  uint8_t yL = Wire.read();
  uint8_t yH = Wire.read();
  uint8_t zL = Wire.read();
  uint8_t zH = Wire.read();
  
  // Combine bytes (little-endian)
  magX = (int16_t)((xH << 8) | xL);
  magY = (int16_t)((yH << 8) | yL);
  magZ = (int16_t)((zH << 8) | zL);
}

void printMagData() {
  static unsigned long lastPrint = 0;
  
  if (millis() - lastPrint < 500) {
    return;  // Print every 500ms
  }
  lastPrint = millis();
  
  // Convert to Gauss (LSB = 1.5 mGauss for LIS2MDL)
  float xGauss = magX * 0.0015f;
  float yGauss = magY * 0.0015f;
  float zGauss = magZ * 0.0015f;
  
  Serial.print("X: ");
  Serial.print(xGauss, 3);
  Serial.print(" G  |  Y: ");
  Serial.print(yGauss, 3);
  Serial.print(" G  |  Z: ");
  Serial.print(zGauss, 3);
  Serial.println(" G");
}

// ============================================================================
// LOW-LEVEL FUNCTIONS
// ============================================================================

void selectMuxChannel(uint8_t channel) {
  if (channel > 7) {
    Serial.println("ERROR: Invalid channel");
    return;
  }
  
  uint8_t control_byte = (1 << channel);
  
  Wire.beginTransmission(MUX_ADDR);
  Wire.write(control_byte);
  Wire.endTransmission();
}

uint8_t readRegister(uint8_t address, uint8_t reg) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  
  if (Wire.endTransmission() != 0) {
    return 0xFF;
  }
  
  if (Wire.requestFrom(address, 1) != 1) {
    return 0xFF;
  }
  
  return Wire.read();
}

void writeRegister(uint8_t address, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}
