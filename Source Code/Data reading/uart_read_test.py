import serial
import struct


def byte_stream_to_int_list(byte_stream):
    # Calculate the number of 16-bit integers
    num_integers = len(byte_stream) // 2
    # Unpack the byte stream into a list of integers
    int_list = struct.unpack('<' + 'H' * num_integers, byte_stream) # TODO will have to change the 'H' character in order to receive formats larger than 16 bit
    return list(int_list)

ser = serial.Serial("COM4", 115200)
while True:
    print("about_to_read")
    bs = ser.read(7)
    print(repr(bs))

    # Example usage
    #int_list = byte_stream_to_int_list(bs)
    #print(int_list) 
