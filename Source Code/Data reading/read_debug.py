import serial
import struct
import threading
import os
import numpy as np
os.chdir(os.path.dirname(os.path.abspath(__file__)))

MAX_SAMPLES = 5700
PORT="COM3"
TEST_TYPE="box full sample"
serial_lock = threading.Lock() # Prevents simultaneous read/write collisions
N=80
def rx_data(ser,lock):
    b=np.zeros(N)
    filling=False
    index=0
    prev=0
    while True:
        with lock:
            if ser.in_waiting >= 3:
                flag = ser.read(1)
                if flag==b'\xf2':
                    data  = ser.read(2)
                    value = struct.unpack('<H', data)[0]
                    a=int(value)
                    if(index==N):
                        index=1
                        print(b)
                        b[0]=a
                    else:
                        b[index]=a
                        index+=1
                    prev=value

if __name__ == "__main__":
    ser = serial.Serial(PORT, 115200,timeout=0.1)
    rx_data(ser,serial_lock)