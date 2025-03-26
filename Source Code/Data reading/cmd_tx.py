import serial
import struct
from matplotlib import pyplot as plt
import pandas as pd

'''

ser = serial.Serial("COM7", 115200)

data_to_send = b'\xF2\x03'  

# Open the serial port if it's not already open
if not ser.is_open:
    ser.open()

# Write data to the serial port
ser.write(data_to_send)


ser.close()
'''

# same as in cmd_rx.cpp
CMD_SET_GAIN_PGA_2 = 0xF1
CMD_SET_GAIN_PGA_3 = 0xF2
CMD_SET_GAIN_CASCADE = 0xF3



class CMD_TX:
    def __init__(self, port: str, baud: str,):
        self.port = port
        self.baud = baud


    def __send_cmd_packet(self, cmd_type, cmd_detail):
        ser = serial.Serial(self.port, self.baud)
        send_bytes = struct.pack('BB', cmd_type, cmd_detail)  

        if not ser.is_open:
            ser.open()
        ser.write(send_bytes)
        ser.close()

        print("Command sent.")


    def set_pga2_gain(self, index):
        """
        Set gain of PGA 2 on STM32f303re.

        Linear Gain is set to 2^index.

        Index must be in [1,4].
        """
        if index not in range(1,5):
            print("set_pga2_gain: gain must be in [1,4]. No command sent.")
            return

        print("Sending command 'set PGA 2 gain to " + str(2**index) + " (linear)'")
        self.__send_cmd_packet(CMD_SET_GAIN_PGA_2, index)

        



    def set_pga3_gain(self, index):
        """
        Set gain of PGA 3 on STM32f303re.

        Linear Gain is set to 2^index.

        Index must be in [1,4].
        """
        if index not in range(1,5):
            print("set_pga3_gain: gain must be in [1,4]. No command sent.")
            return

        print("Sending command 'set PGA 3 gain to " + str(2**index) + " (linear)'")
        self.__send_cmd_packet(CMD_SET_GAIN_PGA_3, index)



    def set_pga_cascade_gain(self, index):
        """
        Set total gain of cascade of PGA 2 & 3 on STM32f303re.

        Requires Blue Robotics external circuitry for total gain to be meaningful.

        Linear Gain is set to 2^index.

        Index must be in [2,8].
        """

        if index not in range(2,9):
            print("set_pga_cascade_gain: gain must be in [2,8]. No command sent.")
            return
        
        print("Sending command 'set PGA cascade gain to " + str(2**index) + " (linear)'")
        self.__send_cmd_packet(CMD_SET_GAIN_CASCADE, index)



if __name__ == "__main__":
    cmd_tx = CMD_TX("COM6", 115200)

    cmd_tx.set_pga_cascade_gain(8)





        