import serial
import struct
import threading
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

MAX_SAMPLES = 5700
PORT="COM7"
TEST_TYPE="box full sample"
def rx_data(ser):
    while True:
        if ser.in_waiting >= 7:
            flag = ser.read(1)
            if flag==b'\xf3':
                payload = ser.read(6)
                char1 = chr(payload[0])
                char2 = chr(payload[1])
                integer_bytes = payload[2:6]
                range_val = int.from_bytes(integer_bytes, byteorder='little')
                print(f"Received: {char1}{char2} | Range: {range_val}")


if __name__ == "__main__":
    ser = serial.Serial(PORT, 115200,timeout=0.1)
    rx_data(ser)