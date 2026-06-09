import serial
import sys

# ================================================================
#  Run this on System2 AFTER pairing with ESP32_BT_Demo
#  via Bluetooth settings
# ================================================================

# CHANGE THIS to your actual COM port:
# Windows --> open Device Manager > Ports > look for Bluetooth Serial > e.g. COM6
# Mac     --> /dev/tty.ESP32_BT_Demo
# Linux   --> /dev/rfcomm0

COM_PORT  = "COM8"       #change
BAUD_RATE = 115200

def main():
    print(f"Trying to connect to ESP32 on {COM_PORT} ...")
    try:
        bt = serial.Serial(COM_PORT, BAUD_RATE, timeout=5)
        print("Connected! Receiving data from ESP32:\n")
    except serial.SerialException as e:
        print(f"\nERROR: Could not open port {COM_PORT}")
        print(f"Reason: {e}")
        print("\nMake sure you have:")
        print("  1. Paired ESP32_BT_Demo in Bluetooth settings")
        print("  2. Correct COM port number in this script")
        sys.exit(1)

    try:
        while True:
            line = bt.readline().decode("utf-8", errors="ignore").strip()
            if line:
                print(line)
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        bt.close()

if __name__ == "__main__":
    main()