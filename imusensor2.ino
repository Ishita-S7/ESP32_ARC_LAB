#include <Wire.h>
#include <LiquidCrystal.h>
#include <math.h>

// LCD Pins
LiquidCrystal lcd(15, 2, 4, 23, 18, 19);

int16_t AcX, AcY, AcZ;
float speed = 0;

void setup()
{
  Serial.begin(115200);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("MPU6050 Ready");

  Wire.begin(21, 22);

  // Wake up MPU6050
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();

  delay(1000);

  lcd.clear();
  lcd.print("Walk Sensor");
}

void loop()
{
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  if (Wire.requestFrom(0x68, 6, true) == 6)
  {
    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();

    float ax = AcX / 16384.0;
    float ay = AcY / 16384.0;
    float az = AcZ / 16384.0;

    // Total acceleration magnitude
    float motion = sqrt(ax * ax + ay * ay + az * az);

    // Remove gravity
    motion = fabs(motion - 1.0);

    // Ignore small vibrations/noise
    if (motion < 0.15)
    {
      motion = 0;
      speed = 0;
    }
    else
    {
      speed = motion * 3.0;
    }

    // Serial Monitor
    Serial.print("Motion: ");
    Serial.print(motion, 3);

    Serial.print("  Speed: ");
    Serial.print(speed, 3);
    Serial.println(" m/s");

    // LCD
    lcd.setCursor(0, 0);
    lcd.print("Speed:");

    lcd.setCursor(0, 1);
    lcd.print("      "); // clear old value
    lcd.setCursor(0, 1);
    lcd.print(speed, 2);
    lcd.print(" m/s");
  }

  delay(100);
}