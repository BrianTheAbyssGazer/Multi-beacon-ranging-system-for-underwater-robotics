import serial
import struct
import threading
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

MAX_SAMPLES = 5700
PORT="COM3"
TEST_TYPE="box full sample"

serial_lock = threading.Lock() # Prevents simultaneous read/write collisions

def rx_data(ser,lock):
    while True:
        with lock:
            if ser.in_waiting >= 2:
                flag = ser.read(1)
                if flag==b'\xf1':
                    raw_data = ser.read(1)
                    try:
                        # Unpack as little-endian signed int
                        value = struct.unpack('<B', raw_data)[0]
                        print(format(value, '08b'))
                    except struct.error:
                        pass
                elif flag==b'\xff':
                    raw_data = ser.read(1)
                    value = struct.unpack('<i', raw_data)[0]
                    print('error')

if __name__ == "__main__":
    ser = serial.Serial(PORT, 115200,timeout=0.1)
    rx_data(ser,serial_lock)