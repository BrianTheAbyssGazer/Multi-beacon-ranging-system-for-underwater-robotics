import serial
import struct
import threading
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

MAX_SAMPLES = 5700
PORT="COM7"
TEST_TYPE="box full sample"
serial_lock = threading.Lock() # Prevents simultaneous read/write collisions
def rx_data(ser,lock):
    N=1
    buffer=bytearray(N) 
    filling=False
    index=0
    template="I am beacon 1!"
    while True:
        with lock:
            if ser.in_waiting >= 2:
                flag = ser.read(1)
                if flag==b'\xf1':
                    char  = ser.read(1)
                    #print( struct.unpack('B', char)[0])
                    #print(format(char[0], '08b'))
                    if not filling:
                        filling = True
                        index=0

                    # 3. Fill the buffer once triggered
                    if filling:
                        buffer[index] = ord(char) # Store as integer (byte)
                        index += 1
                    if index==N:
                        #result_string = buffer.decode('ascii', errors='ignore')
                        #print(f"{result_string}")
                        #xor_binary = [format(ord(a) ^ b, '08b') for a, b in zip(template, buffer)]
                        #print(xor_binary)
                        print(buffer)
                        index=0
                        filling = False

if __name__ == "__main__":
    ser = serial.Serial(PORT, 115200,timeout=0.1)
    rx_data(ser,serial_lock)