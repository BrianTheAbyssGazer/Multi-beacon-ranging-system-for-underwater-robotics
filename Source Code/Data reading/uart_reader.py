import serial
import struct
from matplotlib import pyplot as plt
import pandas as pd





class UARTReader:


    def __init__(self, port: str, baud: str, readCount: int) -> 'UARTReader':
        '''
        Will read readCount number of 16 bit unsigned ints before processing data
        '''
        self.rc = readCount
        self.ser = serial.Serial(port, baud)
        self.all_data = []
        self.chunk_total = 0

        print("Connected on port", port)


    def read_chunk(self): # TODO add try catch so we dont loose data

        bs = self.ser.read(self.rc*2)
        # Unpack the byte stream into a list of integers
        int_list = struct.unpack('<' + 'H' * self.rc, bs) # TODO will have to change the 'H' character in order to receive formats larger than 16 bit
        
        self.all_data += list(int_list)
        self.chunk_total += 1

        print("chunk read complete", self.chunk_total)

        return
    


    def read_frame(self):
        FRAME_START = 2**7
        FRAME_END = 2**6
        FRAME_LEN = 24000 # in unsigned 16 bite integers

        # states for state machine
        WAITING_FOR_FRAME = 0
        READING_FRAME_START_SIGNPOST = 1
        READING_FRAME = 3
        FRAME_END_DETECTED = 4

        state = WAITING_FOR_FRAME

        pastValue = 0
        duplicateCount = 0

        while state == WAITING_FOR_FRAME:

            bs = self.ser.read(1)
            value = int.from_bytes(bs, "big") 

            if value == FRAME_START and pastValue == FRAME_START:
                duplicateCount += 1
            else:
                duplicateCount = 0
            
            if duplicateCount >= 5:
                print("FRAME_START encountered")
                state = READING_FRAME_START_SIGNPOST
                
            pastValue = value
        
        while state == READING_FRAME_START_SIGNPOST:
            bs = self.ser.read(1)
            value = int.from_bytes(bs, "big")
            if value != FRAME_START:
                print("reading frame...")
                state = READING_FRAME
                bs = self.ser.read(1) # flush this byte, so we start 16bit aligned
            
        # read frame:
        bs = self.ser.read(FRAME_LEN*2)
        int_list = struct.unpack('<' + 'H' * FRAME_LEN, bs)
        self.all_data += list(int_list)

        print("frame read complete.")







    def plot_all(self):
        '''
        plot all data read so far
        '''

        plt.figure()
        plt.plot(self.all_data)
        plt.xlabel("sample no.")
        plt.ylabel("16-bit value")
        plt.show()



    def save_all(self, fileName: str):
        '''
        Write out all captured data to csv in "./python/data/<fileName>"
        '''
  
        f = open("./python/data/" + fileName, "w")
        f.write("data\n")

        for num in self.all_data:
            f.write(str(num) + '\n')


        f.close()



    def load_and_plot(fileName: str, show=True) -> None:
        
        df = pd.read_csv(fileName)
        plt.figure()
        plt.plot(df["data"])
        plt.xlabel("sample no.")
        plt.ylabel("ADC value")
        plt.title(fileName)
        

        if (show):
            plt.show()



    def load_and_plot_fancy(fileName: str, sampleFreq: float, title: str, w_start=0, w_end=23999, y_lim=True, show=True) -> None:
        df = pd.read_csv(fileName)
        samplePeriod = (1/sampleFreq)*1e3 # ms
        signal = [df["data"][i] for i in range(len(df["data"])) if (i >= w_start and i < w_end)]
        time = [i*samplePeriod for i in range(len(signal))]
        plt.figure()
        plt.plot(time, signal)
        plt.xlabel("time (ms)")
        plt.ylabel("Signal (12-bit ADC value)")
        plt.title(title)
        if y_lim:
            plt.ylim((0,4000))

        if (show):
            plt.show()





if __name__ == "__main__":

    #UARTReader.load_and_plot("./python/data/BR_pinger_over_air.csv")
    #quit()

    ur = UARTReader("COM7", 115200, 100000)

    ur.read_frame()

    ur.plot_all()

    ur.save_all("set.csv")