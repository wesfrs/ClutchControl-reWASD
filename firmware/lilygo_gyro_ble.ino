// Lilygo T-Display S3 BMI270 Gyro BLE HID Emulator
// Connects via Bluetooth HID + Display shows connection status
// I2C: GPIO 17 = SCL, GPIO 18 = SDA
// Display: ST7789 170x320 via I8080

#include <Wire.h>
#include <TFT_eSPI.h>
#include <BleGamepad.h>

// ==================== CONFIGURATION ====================
#define BMI270_ADDR 0x68
#define CALIBRATION_SAMPLES 200

// Display pins (T-Display S3)
#define TFT_BL 38
#define TFT_CS 6
#define TFT_DC 7
#define TFT_RST 5
#define TFT_MOSI 39
#define TFT_SCLK 8

// ==================== GLOBALS ====================
TFT_eSPI tft = TFT_eSPI(170, 320);
BleGamepad bleGamepad("Lilygo Gyro", "Lilygo", 100);

bool sensorFound = false;
bool calibrated = false;
bool bleConnected = false;
bool wasBleConnected = false;

int16_t gyroOffsetX = 0, gyroOffsetY = 0, gyroOffsetZ = 0;
int32_t calSumX = 0, calSumY = 0, calSumZ = 0;
uint16_t calCount = 0;

struct GyroReport {
  int16_t gyroX;
  int16_t gyroY;
  int16_t gyroZ;
} __attribute__((packed));

GyroReport report = {0, 0, 0};

// ==================== I2C HELPERS ====================
bool i2cWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(BMI270_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

bool i2cRead(uint8_t reg, uint8_t* buf, uint8_t len) {
  Wire.beginTransmission(BMI270_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  Wire.requestFrom(BMI270_ADDR, len);
  for (uint8_t i = 0; i < len; i++) {
    if (Wire.available()) buf[i] = Wire.read();
    else return false;
  }
  return true;
}

uint8_t i2cRead8(uint8_t reg) {
  uint8_t val;
  i2cRead(reg, &val, 1);
  return val;
}

// ==================== DISPLAY HELPERS ====================
void drawConnectionStatus() {
  tft.fillRect(0, 0, 170, 40, TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print("Lilygo Gyro BLE");
  
  tft.setTextSize(2);
  tft.setCursor(10, 50);
  if (bleConnected) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print("BLE: CONNECTED");
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.print("BLE: WAITING...");
  }
  
  // Calibration status
  tft.setCursor(10, 80);
  if (calibrated) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print("Calibrated: OK");
  } else {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.print("Calibrating...");
  }
  
  // Gyro values
  tft.setCursor(10, 110);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(1);
  tft.printf("GX: %6.2f", report.gyroX / 100.0);
  tft.setCursor(10, 125);
  tft.printf("GY: %6.2f", report.gyroY / 100.0);
  tft.setCursor(10, 140);
  tft.printf("GZ: %6.2f", report.gyroZ / 100.0);
  
  // Offset values
  tft.setCursor(10, 160);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.printf("Off X:%5d Y:%5d Z:%5d", gyroOffsetX, gyroOffsetY, gyroOffsetZ);
}

void drawBootScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(20, 130);
  tft.print("Lilygo Gyro");
  tft.setTextSize(2);
  tft.setCursor(30, 170);
  tft.print("BLE HID Starting...");
  delay(1000);
}

// ==================== I2C HELPERS ====================
bool i2cWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(BMI270_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

bool i2cRead(uint8_t reg, uint8_t* buf, uint8_t len) {
  Wire.beginTransmission(BMI270_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  Wire.requestFrom(BMI270_ADDR, len);
  for (uint8_t i = 0; i < len; i++) {
    if (Wire.available()) buf[i] = Wire.read();
    else return false;
  }
  return true;
}

uint8_t i2cRead8(uint8_t reg) {
  uint8_t val;
  i2cRead(reg, &val, 1);
  return val;
}

// ==================== BMI270 INIT ====================
bool bmi270Init() {
  uint8_t chipId = i2cRead8(0x00);
  Serial.printf("BMI270 Chip ID: 0x%02X (expected 0x24)\n", chipId);
  if (chipId != 0x24) return false;
  
  i2cWrite(0x7E, 0xB6); delay(10);  // Soft reset
  i2cWrite(0x7D, 0x03); delay(2);   // Enable accel + gyro
  i2cWrite(0x7C, 0x00); delay(2);   // Normal mode
  i2cWrite(0x41, 0x07);             // Gyro: 100Hz, 2000dps
  i2cWrite(0x40, 0x07 | (0x01 << 4)); // Accel: 100Hz, 4G
  delay(10);
  Serial.println("BMI270 initialized!");
  return true;
}

bool bmi270ReadGyro(int16_t* gx, int16_t* gy, int16_t* gz) {
  uint8_t buf[6];
  Wire.beginTransmission(BMI270_ADDR);
  Wire.write(0x12);
  if (Wire.endTransmission(false) != 0) return false;
  Wire.requestFrom(BMI270_ADDR, 6);
  for (uint8_t i = 0; i < 6; i++) {
    if (Wire.available()) buf[i] = Wire.read();
    else return false;
  }
  *gx = (int16_t)((buf[1] << 8) | buf[0]);
  *gy = (int16_t)((buf[3] << 8) | buf[2]);
  *gz = (int16_t)((buf[5] << 8) | buf[4]);
  return true;
}

// ==================== BLE CALLBACK ====================
class BleConnectionCallbacks : public BleGamepadCallbacks {
  void onConnect() {
    bleConnected = true;
    Serial.println("BLE Connected!");
  }
  void onDisconnect() {
    bleConnected = false;
    Serial.println("BLE Disconnected!");
  }
} bleCallbacks;

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) delay(10);
  
  Serial.println("Lilygo T-Display S3 BMI270 BLE Gyro Starting...");
  
  // Display init
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  
  tft.init();
  tft.setRotation(1);  // Landscape
  tft.fillScreen(TFT_BLACK);
  tft.setTextWrap(false);
  
  drawBootScreen();
  
  // I2C init
  Wire.begin(18, 17);  // SDA=18, SCL=17
  Wire.setClock(400000);
  
  // BMI270 init
  if (!bmi270Init()) {
    sensorFound = false;
    Serial.println("ERROR: BMI270 not found! Check wiring (GPIO 17=SCL, GPIO 18=SDA)");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 100);
    tft.print("BMI270 NOT FOUND!");
  } else {
    sensorFound = true;
    Serial.println("BMI270 found and initialized!");
  }
  
  // BLE Gamepad init
  bleGamepad.setCallbacks(&bleCallbacks);
  BleGamepadConfiguration cfg;
  cfg.setAutoReport(false);  // Manual report sending
  bleGamepad.begin(&cfg);
  
  Serial.println("BLE Gamepad started. Waiting for connection...");
  Serial.println("Setup complete!");
  if (sensorFound) {
    Serial.println("Calibrating... Keep sensor STILL on flat surface for 2 seconds...");
  }
}

// ==================== CALIBRATION ====================
bool sensorFound = false;
bool calibrated = false;
int16_t gyroOffsetX = 0, gyroOffsetY = 0, gyroOffsetZ = 0;
int32_t calSumX = 0, calSumY = 0, calSumZ = 0;
uint16_t calCount = 0;

struct GyroReport {
  int16_t gyroX;
  int16_t gyroY;
  int16_t gyroZ;
} __attribute__((packed));

GyroReport report = {0, 0, 0};
int32_t calSumX = 0, calSumY = 0, calSumZ = 0;
uint16_t calCount = 0;

void runCalibration() {
  if (!sensorFound || calibrated) return;
  
  int16_t gx, gy, gz;
  if (!bmi270ReadGyro(&gx, &gy, &gz)) return;
  
  int16_t gx_dps100 = (int16_t)(gx * 6.1035f);
  int16_t gy_dps100 = (int16_t)(gy * 6.1035f);
  int16_t gz_dps100 = (int16_t)(gz * 6.1035f);
  
  calSumX += gx_dps100;
  calSumY += gy_dps100;
  calSumZ += gz_dps100;
  calCount++;
  
  if (calCount >= CALIBRATION_SAMPLES) {
    gyroOffsetX = calSumX / CALIBRATION_SAMPLES;
    gyroOffsetY = calSumY / CALIBRATION_SAMPLES;
    gyroOffsetZ = calSumZ / CALIBRATION_SAMPLES;
    
    calibrated = true;
    Serial.println("Calibration complete!");
    Serial.printf("Offsets: X=%d, Y=%d, Z=%d (x0.01 deg/s)\n", 
                  gyroOffsetX, gyroOffsetY, gyroOffsetZ);
  }
}

bool bmi270ReadGyro(int16_t* gx, int16_t* gy, int16_t* gz) {
  uint8_t buf[6];
  Wire.beginTransmission(BMI270_ADDR);
  Wire.write(0x12);
  if (Wire.endTransmission(false) != 0) return false;
  Wire.requestFrom(BMI270_ADDR, 6);
  for (uint8_t i = 0; i < 6; i++) {
    if (Wire.available()) buf[i] = Wire.read();
    else return false;
  }
  *gx = (int16_t)((buf[1] << 8) | buf[0]);
  *gy = (int16_t)((buf[3] << 8) | buf[2]);
  *gz = (int16_t)((buf[5] << 8) | buf[4]);
  return true;
}

// ==================== MAIN LOOP ====================
void loop() {
  // Update display every 100ms
  static uint32_t lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate >= 100) {
    lastDisplayUpdate = millis();
    drawConnectionStatus();
  }
  
  // Check BLE connection changes
  if (bleConnected != wasBleConnected) {
    wasBleConnected = bleConnected;
    drawConnectionStatus();
  }
  
  if (!calibrated) {
    runCalibration();
    delay(10);
    return;
  }
  
  // Read gyro at 100Hz
  static uint32_t lastReport = 0;
  if (millis() - lastReport >= 10) {
    lastReport = millis();
    
    if (sensorFound && bleConnected) {
      int16_t gx, gy, gz;
      if (bmi270ReadGyro(&gx, &gy, &gz)) {
        // Convert to dps * 100 and apply calibration
        report.gyroX = (int16_t)(gx * 6.1035f) - gyroOffsetX;
        report.gyroY = (int16_t)(gy * 6.1035f) - gyroOffsetY;
        report.gyroZ = (int16_t)(gz * 6.1035f) - gyroOffsetZ;
        
        // Send via BLE HID (right stick = gyro)
        // Map gyro to right stick axes
        int16_t stickX = constrain(report.gyroX / 20, -32767, 32767);
        int16_t stickY = constrain(report.gyroY / 20, -32767, 32767);
        
        bleGamepad.setAxes(0, 0, stickX, stickY, 0, 0);
        bleGamepad.sendReport();
      }
    }
  }
  
  delay(1);
}