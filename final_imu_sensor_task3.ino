#include <Wire.h>
#include <math.h>
#include <LiquidCrystal.h>

const int MPU = 0x68;
LiquidCrystal lcd(15, 2, 4, 23, 18, 19);

const float ACC_SCALE      = 16384.0;
const float GYRO_SCALE     = 131.0;
const float GYRO_THRESHOLD = 0.5;
const float ACC_THRESHOLD  = 0.02;
const int   N              = 500;
const float ROOM_TEMP      = 26.0;

int16_t AX, AY, AZ, GX, GY, GZ, TEMP_RAW;
float temperature, ax, ay, az, gx, gy, gz;
float offsetX = 0, offsetY = 0, offsetZ = 0, tempOffset = 0;

int page = 0;
unsigned long lastSwitch = 0;
const unsigned long PAGE_INTERVAL = 2000;

// ─────────────────────────────────────────
void readIMU() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 8, true);
  AX       = Wire.read() << 8 | Wire.read();
  AY       = Wire.read() << 8 | Wire.read();
  AZ       = Wire.read() << 8 | Wire.read();
  TEMP_RAW = Wire.read() << 8 | Wire.read();

  temperature = ((TEMP_RAW / 340.0) + 36.53) - tempOffset;

  ax = AX / ACC_SCALE;
  ay = AY / ACC_SCALE;
  az = AZ / ACC_SCALE;

  // Dead zone clamp
  ax = (abs(ax) < ACC_THRESHOLD) ? 0.0 : ax;
  ay = (abs(ay) < ACC_THRESHOLD) ? 0.0 : ay;
  // Don't clamp az — it always shows ~1g due to gravity

  Wire.beginTransmission(MPU);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);
  GX = Wire.read() << 8 | Wire.read();
  GY = Wire.read() << 8 | Wire.read();
  GZ = Wire.read() << 8 | Wire.read();

  gx = (GX - offsetX) / GYRO_SCALE;
  gy = (GY - offsetY) / GYRO_SCALE;
  gz = (GZ - offsetZ) / GYRO_SCALE;

  gx = (abs(gx) < GYRO_THRESHOLD) ? 0.0 : gx;
  gy = (abs(gy) < GYRO_THRESHOLD) ? 0.0 : gy;
  gz = (abs(gz) < GYRO_THRESHOLD) ? 0.0 : gz;
}

// ─────────────────────────────────────────
void updateLCD() {
  lcd.clear();
  switch (page) {

    case 0:
      // Page 1 — Temperature
      lcd.setCursor(0, 0);
      lcd.print("Temp:");
      lcd.print(temperature, 1);
      lcd.print((char)223);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("-- IMU ACTIVE --");
      break;

    case 1:
      // Page 2 — Accel X and Y
      lcd.setCursor(0, 0);
      lcd.print("AX:");
      lcd.print(ax, 3);
      lcd.print("g");
      lcd.setCursor(0, 1);
      lcd.print("AY:");
      lcd.print(ay, 3);
      lcd.print("g");
      break;

    case 2:
      // Page 3 — Accel Z and Gyro X
      lcd.setCursor(0, 0);
      lcd.print("AZ:");
      lcd.print(az, 3);
      lcd.print("g");
      lcd.setCursor(0, 1);
      lcd.print("GX:");
      lcd.print(gx, 2);
      lcd.print(" dps");
      break;

    case 3:
      // Page 4 — Gyro Y and Z
      lcd.setCursor(0, 0);
      lcd.print("GY:");
      lcd.print(gy, 2);
      lcd.print(" dps");
      lcd.setCursor(0, 1);
      lcd.print("GZ:");
      lcd.print(gz, 2);
      lcd.print(" dps");
      break;
  }
}

// ─────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  lcd.begin(16, 2);

  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  lcd.print("Calibrating...");

  long sumX = 0, sumY = 0, sumZ = 0;
  float tempSum = 0;

  for (int i = 0; i < N; i++) {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    GX = Wire.read() << 8 | Wire.read();
    GY = Wire.read() << 8 | Wire.read();
    GZ = Wire.read() << 8 | Wire.read();
    sumX += GX;
    sumY += GY;
    sumZ += GZ;

    Wire.beginTransmission(MPU);
    Wire.write(0x41);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 2, true);
    TEMP_RAW = Wire.read() << 8 | Wire.read();
    tempSum += (TEMP_RAW / 340.0) + 36.53;

    delay(5);
  }

  offsetX    = sumX / (float)N;
  offsetY    = sumY / (float)N;
  offsetZ    = sumZ / (float)N;
  tempOffset = (tempSum / N) - ROOM_TEMP;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  IMU  READY  ");
  lcd.setCursor(0, 1);
  lcd.print("  MPU-6050 OK ");
  delay(1500);

  lastSwitch = millis();
  updateLCD();
}

// ─────────────────────────────────────────
void loop() {
  readIMU();

  if (millis() - lastSwitch >= PAGE_INTERVAL) {
    page = (page + 1) % 4;   // now 4 pages
    lastSwitch = millis();
    updateLCD();
  }

  Serial.print("T:");   Serial.print(temperature, 1);
  Serial.print(" AX:"); Serial.print(ax, 3);
  Serial.print(" AY:"); Serial.print(ay, 3);
  Serial.print(" AZ:"); Serial.print(az, 3);
  Serial.print(" GX:"); Serial.print(gx, 2);
  Serial.print(" GY:"); Serial.print(gy, 2);
  Serial.print(" GZ:"); Serial.println(gz, 2);

  delay(100);
}