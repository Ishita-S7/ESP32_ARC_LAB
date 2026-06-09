import serial
import sys
import os
import time

COM_PORT  = "COM3"    # <-- COM port of ESP32 on System 1
BAUD_RATE = 115200

def send_image(port, baud, image_path):
    if not os.path.exists(image_path):
        print(f"ERROR: File not found: {image_path}")
        sys.exit(1)

    print(f"Connecting to ESP32 on {port} ...")
    try:
        ser = serial.Serial(port, baud, timeout=10)
        time.sleep(2)  # wait for ESP32 to boot/reset
        print("Connected!")
    except serial.SerialException as e:
        print(f"ERROR: {e}")
        sys.exit(1)

    with open(image_path, "rb") as f:
        image_data = f.read()

    file_size = len(image_data)
    print(f"Sending: {image_path} ({file_size} bytes)")

    # Step 1: Send IMG_START signal
    ser.write(b"IMG_START\n")
    time.sleep(0.1)

    # Step 2: Send file size
    ser.write(f"{file_size}\n".encode("utf-8"))
    time.sleep(0.1)

    # Step 3: Send raw image bytes in chunks
    chunk_size = 512
    sent = 0
    while sent < file_size:
        chunk = image_data[sent:sent + chunk_size]
        ser.write(chunk)
        sent += len(chunk)
        percent = int((sent / file_size) * 100)
        print(f"  Progress: {sent}/{file_size} bytes ({percent}%)", end="\r")
        time.sleep(0.01)  # small delay to avoid buffer overflow

    print(f"\n\nDone! Sent {sent} bytes.")
    ser.close()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python sender.py <image_path>")
        print("Example: python sender.py dog.jpg")
        sys.exit(1)

    send_image(COM_PORT, BAUD_RATE, sys.argv[1])