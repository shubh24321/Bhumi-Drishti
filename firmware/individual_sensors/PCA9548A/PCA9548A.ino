#include <Wire.h>

// --- Configuration ---
#define MUX_ADDR 0x70      // Address with A0, A1, A2 to GND
#define RESET_PIN 4        // RST Pin with 10k pull-up to 3.3V
#define CHECK_INTERVAL 5000 

unsigned long lastCheck = 0;

void setup() {
  // Teensy 4.1 Serial is very fast, no baud rate required but 115200 is standard
  Serial.begin(115200);
  
  // wait for Serial Monitor
  while (!Serial && millis() < 3000);

  Serial.println("\n--- PCA9548A Hardware Test (Pull-Up Verified) ---");

  // 1. Initialize the Reset Pin
  // The 10k resistor holds this HIGH. We set it to OUTPUT to control it.
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH); 
  
  // 2. Start I2C on Pins 18 (SDA) and 19 (SCL)
  Wire.begin();
  
  // Optional: Increase I2C speed to 400kHz to test pull-up strength
  // Stronger pull-ups (2.2k) handle 400kHz better than internal ones.
  Wire.setClock(400000); 

  Serial.println("System Ready. Checking connection every 5s...");
}

void loop() {
  unsigned long now = millis();

  if (now - lastCheck >= CHECK_INTERVAL) {
    lastCheck = now;

    Serial.print("[Time: ");
    Serial.print(now / 1000);
    Serial.print("s] Scanning for Mux... ");

    // Perform an I2C discovery "ping"
    Wire.beginTransmission(MUX_ADDR);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.println("SUCCESS: PCA9548A is ONLINE at 0x70");
      Serial.println("-> Pull-up resistors are working correctly.");
    } 
    else if (error == 4) {
      Serial.println("HARDWARE ERROR 4: Bus Error.");
      Serial.println("-> Check if 2.2k resistors are correctly tied to 3.3V.");
    } 
    else {
      Serial.print("FAILED: Error Code ");
      Serial.println(error);
      Serial.println("-> Check A0/A1/A2 and Power connections.");
    }
    
    Serial.println("------------------------------------------------");
  }
}
