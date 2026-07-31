import serial
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

MAX_SAMPLES = 5700
PORT="COM8"
TEST_TYPE="box full sample"
def rx_data(ser):
    while True:
        if ser.in_waiting >= 7: # 1 reading consists of 7 bytes
            flag = ser.read(1) # first byte is a flag
            if flag==b'\xf3':
                payload = ser.read(6)
                if(0):
                    # this is for actual range reading
                    integer_bytes = payload[2:6]  # the raw time of fight (in number of clock cycles of the stm32 timer)
                    tof_val = int.from_bytes(integer_bytes, byteorder='little')
                    #tof_val-=6*12288 # bias/offset to be calibrated 
                    #range_val=float(tof_val)*13.043/12.0 
                    print(f"ID: {char1}{char2} | Range: {range_val}")
                else:
                    # this is for debugging
                    char1 = chr(payload[0])
                    char2 = chr(payload[1])
                    amplitude_byte = payload[2:4]
                    gain_byte = payload[4:6]
                    amplitude = int.from_bytes(amplitude_byte, byteorder='little')
                    gain = int.from_bytes(gain_byte, byteorder='little')
                    print(f"ID: {char1}{char2} | amplitude: {amplitude} | gain: {gain}")

if __name__ == "__main__":
    ser = serial.Serial(PORT, 115200,timeout=0.1)
    rx_data(ser)