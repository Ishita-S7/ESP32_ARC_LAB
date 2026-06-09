import serial
import os
import sys
import time
import struct

VIDEO_PATH = r"C:\Users\Ishita\OneDrive\Desktop\Python\kuka\hambuger.mp4"
ESP32_PORT = "COM4"
BAUD_RATE  = 115200
CHUNK_SIZE = 512

def main():
    if not os.path.exists(VIDEO_PATH):
        print(f"ERROR: Video not found: {VIDEO_PATH}")
        sys.exit(1)

    video_size = os.path.getsize(VIDEO_PATH)
    print(f"Video : {os.path.basename(VIDEO_PATH)}")
    print(f"Size  : {video_size} bytes ({video_size/1024/1024:.2f} MB)")

    print(f"\nConnecting to ESP32 on {ESP32_PORT}...")
    try:
        esp = serial.Serial(ESP32_PORT, BAUD_RATE, timeout=10)
        time.sleep(2)
        esp.reset_input_buffer()
    except serial.SerialException as e:
        print(f"ERROR: {e}")
        sys.exit(1)

    print("Connected!")
    print("Waiting 10 seconds for receiver to get ready...")
    print(">>> Make sure video_receiver.py is running on Shreya's laptop NOW <<<\n")

    for i in range(10, 0, -1):
        print(f"  Starting in {i}...", end="\r")
        time.sleep(1)
    print("\nSending now!\n")

    # Send magic marker + file size
    esp.write(b"VID:")
    esp.write(struct.pack('>I', video_size))
    time.sleep(0.1)

    bytes_sent = 0
    start_time = time.time()

    with open(VIDEO_PATH, "rb") as f:
        while True:
            chunk = f.read(CHUNK_SIZE)
            if not chunk:
                break
            esp.write(chunk)
            bytes_sent += len(chunk)

            elapsed  = time.time() - start_time
            speed_kb = (bytes_sent / 1024) / elapsed if elapsed > 0 else 0
            percent  = (bytes_sent / video_size) * 100
            print(f"  Sent: {bytes_sent}/{video_size} bytes ({percent:.1f}%) | {speed_kb:.1f} KB/s", end="\r")

    elapsed = time.time() - start_time
    print(f"\n\nDone! {bytes_sent} bytes in {elapsed:.1f}s ({bytes_sent/1024/elapsed:.1f} KB/s avg)")
    esp.close()

if __name__ == "__main__":
    main()