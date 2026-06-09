#include <BluetoothSerial.h>

BluetoothSerial BT;

#define USB_BAUD       115200
#define BT_DEVICE_NAME "ESP32_Bridge"   // must match COM8 pairing on System 2

// State machine
enum State { WAIT_START, WAIT_SIZE, RECEIVE_DATA };
State state = WAIT_START;

String lineBuffer = "";
size_t expectedSize = 0;
size_t receivedBytes = 0;

void setup() {
  Serial.begin(USB_BAUD);   // USB Serial ← System 1
  BT.begin(BT_DEVICE_NAME); // Bluetooth  → System 2
  Serial.println("ESP32 Bridge ready.");
  Serial.println("Waiting for image from System 1...");
}

void loop() {
  // ── USB Serial → Bluetooth forwarding ──────────────────────────────────
  if (Serial.available()) {

    if (state == WAIT_START || state == WAIT_SIZE) {
      // Read line-by-line for control signals
      char c = Serial.read();
      if (c == '\n') {
        lineBuffer.trim();

        if (state == WAIT_START && lineBuffer == "IMG_START") {
          BT.println("IMG_START");   // forward signal to System 2
          Serial.println("[Bridge] IMG_START forwarded");
          state = WAIT_SIZE;

        } else if (state == WAIT_SIZE) {
          expectedSize  = lineBuffer.toInt();
          receivedBytes = 0;
          BT.println(lineBuffer);    // forward size to System 2
          Serial.printf("[Bridge] Size forwarded: %u bytes\n", expectedSize);
          state = RECEIVE_DATA;
        }

        lineBuffer = "";
      } else {
        lineBuffer += c;
      }

    } else if (state == RECEIVE_DATA) {
      // Forward raw bytes directly over Bluetooth
      uint8_t buf[512];
      size_t remaining = expectedSize - receivedBytes;
      size_t toRead    = min((size_t)Serial.available(), min((size_t)512, remaining));

      size_t bytesRead = Serial.readBytes(buf, toRead);
      BT.write(buf, bytesRead);
      receivedBytes += bytesRead;

      if (receivedBytes >= expectedSize) {
        Serial.println("[Bridge] Transfer complete!");
        state = WAIT_START;
        lineBuffer    = "";
        expectedSize  = 0;
        receivedBytes = 0;
      }
    }
  }
}
}
