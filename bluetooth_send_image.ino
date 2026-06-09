#include "BluetoothSerial.h"

// This creates a Bluetooth serial object
// System2 will see the device name "ESP32_BT_Demo" when scanning
BluetoothSerial SerialBT;

int counter = 0;

void setup() {
  Serial.begin(115200);               // USB serial (for debugging on System1)
  SerialBT.begin("ESP32_BT_Demo");    // Bluetooth device name System2 will search for
  Serial.println("ESP32 Bluetooth started. Waiting for System2 to connect...");
}

void loop() {
  counter++;

  // Build the message
  String message = "Reading #" + String(counter) + 
                   " | Temp: "  + String(random(25, 40)) + " C" +
                   " | Status: OK";

  // Send over Bluetooth to System2
  if (SerialBT.hasClient()) {
    SerialBT.println(message);
    Serial.println("BT SENT: " + message);   // Also print on USB so System1 can verify
  } else {
    Serial.println("Waiting for System2 to connect via Bluetooth...");
  }

  delay(1000);  // Send one reading every second
}
