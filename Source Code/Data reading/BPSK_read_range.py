import serial
import struct
import threading
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

MAX_SAMPLES = 5700
PORT="COM5"
TEST_TYPE="box full sample"
def rx_data(ser):
    while True:
        if ser.in_waiting >= 7:
            flag = ser.read(1)
            if flag==b'\xf3':
                payload = ser.read(6)
                char1 = chr(payload[0])
                char2 = chr(payload[1])
                if(0):
                    integer_bytes = payload[2:6]
                    range_val = int.from_bytes(integer_bytes, byteorder='little')
                    range_val-=6*12288
                    #dist=float(range_val)*13.043/12.0
                    print(f"ID: {char1}{char2} | Range: {range_val}")
                else:
                    amplitude_byte = payload[2:4]
                    gain_byte = payload[4:6]
                    amplitude = int.from_bytes(amplitude_byte, byteorder='little')
                    gain = int.from_bytes(gain_byte, byteorder='little')
                    print(f"ID: {char1}{char2} | Amp: {amplitude} | gain: {gain}")

if __name__ == "__main__":
    ser = serial.Serial(PORT, 115200,timeout=0.1)
    rx_data(ser)