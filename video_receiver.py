import serial
import sys
import os
import time
import struct

COM_PORT    = "COM11"
BAUD_RATE   = 115200
SAVE_FOLDER = r"C:\Users\Ishita\OneDrive\Desktop\Python\kuka"  # <-- change if needed
SAVE_NAME   = "received_video.mp4"

def read_exactly(port, n):
    buf = bytearray()
    while len(buf) < n:

        chunk = port.read(n - len(buf))
        if chunk:
            buf.extend(chunk)
    return bytes(buf)

def main():
    os.makedirs(SAVE_FOLDER, exist_ok=True)
    save_path = os.path.join(SAVE_FOLDER, SAVE_NAME)

    print(f"Connecting to ESP32 Bluetooth on {COM_PORT}...")
    try:
        bt = serial.Serial(COM_PORT, BAUD_RATE, timeout=60)
        time.sleep(2)
        bt.reset_input_buffer()
        print("Connected!\n")
    except serial.SerialException as e:
        print(f"ERROR: {e}")
        sys.exit(1)

    print("Waiting for transfer header (tell your friend to start sender now)...")

    magic = read_exactly(bt, 4)
    if magic != b"VID:":
        print(f"ERROR: Bad magic bytes: {magic!r}  (expected b'VID:')")
        print("  → Make sure sender is using the updated script!")
        bt.close()
        sys.exit(1)

    size_bytes    = read_exactly(bt, 4)
    expected_size = struct.unpack('>I', size_bytes)[0]
    print(f"File size: {expected_size} bytes ({expected_size/1024/1024:.2f} MB)")
    print("Receiving...\n")

    video_data = bytearray()
    start_time = time.time()

    try:
        while len(video_data) < expected_size:
            remaining = expected_size - len(video_data)
            chunk     = bt.read(min(512, remaining))
            if chunk:
                video_data.extend(chunk)
                elapsed  = time.time() - start_time
                speed_kb = (len(video_data) / 1024) / elapsed if elapsed > 0 else 0
                percent  = (len(video_data) / expected_size) * 100
                print(f"  {len(video_data)}/{expected_size} bytes ({percent:.1f}%) | {speed_kb:.1f} KB/s", end="\r")

        with open(save_path, "wb") as f:
            f.write(video_data)

        elapsed = time.time() - start_time
        print(f"\n\n✅ Video saved!")
        print(f"   File  : {save_path}")
        print(f"   Size  : {len(video_data)} bytes")
        print(f"   Time  : {elapsed:.1f}s ({len(video_data)/1024/elapsed:.1f} KB/s avg)")

    except KeyboardInterrupt:
        print("\nStopped by user.")
        if video_data:
            partial = os.path.join(SAVE_FOLDER, "partial_" + SAVE_NAME)
            with open(partial, "wb") as f:
                f.write(video_data)
            print(f"Partial file saved: {partial}")
    finally:
        bt.close()

if __name__ == "__main__":
    main()