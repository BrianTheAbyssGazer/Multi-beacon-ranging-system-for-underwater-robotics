import serial
import struct
from matplotlib import pyplot as plt
import pandas as pd



class IndexInfoPacket:
    INDEX_HEADER  = 0xF0 #6 byte payload with buf_idx, pre_idx, peak_val
    INFO_1_HEADER = 0xF1 #reserved
    INFO_2_HEADER = 0xF2 #reserved
    INFO_3_HEADER = 0xF3 #reserved
    ERR_1_HEADER  = 0xF8 #indicates next buffer is full before we have finished processing the previous
    ERR_2_HEADER  = 0xF9 #reserved
    ERR_3_HEADER  = 0xFA #reserved


    def __init__(self, packet_header, buf_idx : int, pre_idx : int, peak_val : int) -> 'IndexInfoPacket':
        self.packet_header = packet_header
        self.buf_idx = buf_idx
        self.pre_idx = pre_idx
        self.peak_val = peak_val

    def __str__(self) -> str:
        if   self.packet_header == IndexInfoPacket.INDEX_HEADER:
            return "IndexInfoPacket: | idx=" + str(self.buf_idx) + " | pfx=" + str(self.pre_idx) + " | val=" + str(self.peak_val) + " |"
        elif self.packet_header == IndexInfoPacket.ERR_1_HEADER:
            return "IndexInfoPacket: ERROR 1 - Buffer filled before previous buffer processed. | idx=" + str(self.buf_idx) + " | pfx=" + str(self.pre_idx) + " |"
        elif self.packet_header == IndexInfoPacket.ERR_2_HEADER:
            return "IndexInfoPacket: ERROR 2 - reserved."
        elif self.packet_header == IndexInfoPacket.ERR_3_HEADER:
            return "IndexInfoPacket: ERROR 3 - reserved."
        elif self.packet_header == IndexInfoPacket.INFO_1_HEADER:
            return "IndexInfoPacket: INFO 1 - reserved."
        elif self.packet_header == IndexInfoPacket.INFO_2_HEADER:
            return "IndexInfoPacket: INFO 2 - reserved."
        elif self.packet_header == IndexInfoPacket.INFO_3_HEADER:
            return "IndexInfoPacket: INFO 3 - reserved."
        else:
            return "IndexInfoPacket: BAD PACKET - header=" + str(self.packet_header)
        


class IndexInfoRX:

    IN_BUFFER_LEN = 18000

    def __init__(self, port: str, baud: str) -> 'IndexInfoRX':
        self.ser = serial.Serial(port, baud)
        print("IndexInfoRX connected.")


    def read_next_packet(self) -> IndexInfoPacket:

        #states
        SEARCH_PACKET_START = 0
        POTENTIAL_PACKET = 1
        state = SEARCH_PACKET_START

        # bytes
        byte_arr = [0,0,0,0,0,0,0]

        # note STM32fr303e is little endian (LSB first)

        while True:
            if state == SEARCH_PACKET_START: #_------------------------------------------
                bs = self.ser.read(1)
                byte_arr[0] = int.from_bytes(bs, "big")
                if byte_arr[0] > (1<<7):
                    state = POTENTIAL_PACKET

            elif state == POTENTIAL_PACKET:  #_------------------------------------------
                packet = True
                for i in range(1,7):
                    bs = self.ser.read(1)
                    byte_arr[i] = int.from_bytes(bs, "big")
                    if i % 2 == 0: #this byte should be the MSB (less than 1<<15)
                        if byte_arr[i] >= (1<<7):
                            packet = False
                            byte_arr[0] = byte_arr[i]
                            break
                
                if (packet):
                    return IndexInfoPacket(byte_arr[0], byte_arr[1] + byte_arr[2]*(1<<8), 
                                                        byte_arr[3] + byte_arr[4]*(1<<8),
                                                        byte_arr[5] + byte_arr[6]*(1<<8))
                    

    def pipe_to_file(self, filename :str, num_packets :int):
        f = open("./python/data/" + filename, "w")
        f.write("idx,pfx,val\n")

        for i in range(num_packets):
            if i%100==0: print("Reading Packet", i, "of", num_packets)
            p = self.read_next_packet()
            if p.packet_header == IndexInfoPacket.INDEX_HEADER:
                f.write(str(p.buf_idx)+','+str(p.pre_idx)+","+str(p.peak_val)+"\n")

        f.close()


    def plot_all_from_file(fileName:str, show=True):
        print("Plotting from file...")
        df = pd.read_csv("./python/data/" + fileName)
        plt.figure()
        plt.plot(df["idx"], label="idx")
        plt.plot(df["pfx"], label="pfx")
        plt.plot(df["val"], label="val")
        plt.legend()
        plt.xlabel("peak no.")
        #plt.ylabel("ADC value")
        plt.title(fileName)

        if (show):
            plt.show()
    
    def plot_val_from_file(fileName:str, show=True):
        print("Plotting from file...")
        df = pd.read_csv("./python/data/" + fileName)
        plt.figure()
        plt.plot(df["val"])
        plt.xlabel("peak no.")
        plt.ylabel("ADC value")
        plt.title(fileName)
        plt.ylim((0,4096))

        if (show):
            plt.show()
    

    def plot_full_idx_from_file(fileName:str, show=True):
        print("Plotting from file...")
        df = pd.read_csv("./python/data/" + fileName)
        full_idxs = []
        for i in range(len(df["idx"])):
            full_idxs.append(df["idx"][i] + df["pfx"][i]*IndexInfoRX.IN_BUFFER_LEN)

        plt.figure()
        plt.plot(full_idxs)
        plt.xlabel("Peak number")
        plt.ylabel("Index")
        plt.title(fileName)

        if (show):
            plt.show()


    def plot_diffs_from_file(fileName:str, show=True):
        print("Plotting from file...")
        df = pd.read_csv("./python/data/" + fileName)
        full_idxs = []
        for i in range(len(df["idx"])):
            full_idxs.append(df["idx"][i] + df["pfx"][i]*IndexInfoRX.IN_BUFFER_LEN)
        diffs = []
        for i in range(1,len(full_idxs)):
            diffs.append(full_idxs[i] - full_idxs[i-1])


        plt.figure()
        plt.plot(diffs)
        plt.xlabel("Peak number")
        plt.ylabel("Index delta")
        plt.title(fileName)
        
        if (show):
            plt.show()

if __name__ == "__main__":
    '''
    rx = IndexInfoRX("COM7", 115200)

    for i in range(1000):
        print(rx.read_next_packet())
    quit()
    #'''

    file = "basic_peak_search_test_with_new_adc_sample_rate.csv"
    rx = IndexInfoRX("COM9", 115200)
    rx.pipe_to_file(file, 10)
    #IndexInfoRX.plot_all_from_file(file, show=False)
    #IndexInfoRX.plot_full_idx_from_file(file, show=False)
    IndexInfoRX.plot_val_from_file(file, show=False)
    IndexInfoRX.plot_diffs_from_file(file)
    
    