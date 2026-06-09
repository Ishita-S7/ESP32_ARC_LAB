import serial
import sys
import os
from datetime import datetime
 
COM_PORT  = "COM8"    # <-- change to your working COM port
BAUD_RATE = 115200
 
def main():
    print(f"Connecting to ESP32 on {COM_PORT} ...")
    try:
        bt = serial.Serial(COM_PORT, BAUD_RATE, timeout=15)
        print("Connected! Waiting for image transmission...\n")
    except serial.SerialException as e:
        print(f"ERROR: {e}")
        sys.exit(1)
 
    image_data   = bytearray()
    receiving    = False
    expected_size = 0
 
    try:
        while True:
            if not receiving:
                # Read text lines for control signals
                line = bt.readline().decode("utf-8", errors="ignore").strip()
                if not line:
                    continue
                print(f"Signal: {line}")
 
                if line == "IMG_START":
                    # Next line is the file size
                    size_line = bt.readline().decode("utf-8", errors="ignore").strip()
                    expected_size = int(size_line)
                    image_data = bytearray()
                    receiving  = True
                    print(f"Receiving image ({expected_size} bytes)...")
 
            else:
                # Read raw bytes until we have the full image
                remaining = expected_size - len(image_data)
                chunk = bt.read(min(512, remaining))
                image_data.extend(chunk)
 
                percent = int((len(image_data) / expected_size) * 100)
                print(f"  Progress: {len(image_data)}/{expected_size} bytes ({percent}%)", end="\r")
 
                if len(image_data) >= expected_size:
                    receiving = False
                    filename  = f"received_dog_{datetime.now().strftime('%H%M%S')}.jpg"
                    with open(filename, "wb") as f:
                        f.write(image_data)
                    print(f"\n\nImage saved as: {filename}")
                    print(f"Size: {len(image_data)} bytes")
                    print(f"Location: {os.path.abspath(filename)}")
                    print("\nWaiting for next transmission...")
 
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        bt.close()
 
if __name__ == "__main__":
    main()
 