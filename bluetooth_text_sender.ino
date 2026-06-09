#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// Target name must match the sender's local name exactly
String targetDevice = "WALKING_TRACKER_NODE";
bool connected = false;

void setup() {
  Serial.begin(115200);
  
  // Initialize Master Bluetooth mode
  SerialBT.begin("LAPTOP_RECEIVER_HUB", true); 
  Serial.println("Receiver Active. Initiating direct link to target board...");
  
  // Actively search and connect to the walker's ESP32
  Serial.print("Searching for " + targetDevice + "... ");
  connected = SerialBT.connect(targetDevice);
  
  if(connected) {
    Serial.println("CONNECTED SUCCESSFULLY!");
  } else {
    Serial.println("CONNECTION FAILED. Retrying in loop...");
  }
  
  Serial.println("\n=====================================================================");
  Serial.println("                    LIVE GAIT SPEED TELEMETRY                        ");
  Serial.println("=====================================================================");
}

void loop() {
  // If connection dropped, try to auto-reconnect
  if (!connected) {
    Serial.println("Attempting reconnection link...");
    connected = SerialBT.connect(targetDevice);
    if (connected) Serial.println("RECONNECTED!");
    delay(2000);
    return;
  }

  // Parse incoming wireless stream
  if (SerialBT.available()) {
    String incoming = SerialBT.readStringUntil('\n');
    incoming.trim();
    
    if(incoming.startsWith("[") && incoming.endsWith("]")) {
      incoming = incoming.substring(1, incoming.length() - 1);
      
      int firstComma = incoming.indexOf(',');
      int secondComma = incoming.indexOf(',', firstComma + 1);
      
      if(firstComma != -1 && secondComma != -1) {
        String intervalSteps = incoming.substring(0, firstComma);
        String totalSteps = incoming.substring(firstComma + 1, secondComma);
        String speed = incoming.substring(secondComma + 1);
        
        Serial.print("INTERVAL STEPS: " + intervalSteps);
        Serial.print(" | TOTAL STEPS: " + totalSteps);
        Serial.println(" | SPEED: " + speed + " m/s");
      }
    }
  }
  delay(10);
}