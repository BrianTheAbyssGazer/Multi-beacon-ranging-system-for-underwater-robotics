from index_info_rx import IndexInfoRX
from index_info_rx import IndexInfoPacket


from matplotlib import pyplot as plt
import pandas as pd
#from pynput.keyboard import Key, Listener



class RangeCalc:
    def __init__(self, port: str, baud: str, ping_period: int, offset: float, adc_buf_len=18000, adc_sample_freq=1.38e6, speed_of_sound=1500.0) -> None:
        '''
        * ping_period should have units of total buffer temporal length.
        * offset should be in meters.
        '''
        self.iirx = IndexInfoRX(port, baud)
        self.ping_period = ping_period
        self.sos = speed_of_sound
        self.sample_freq = adc_sample_freq
        self.buf_len = adc_buf_len
        self.offset = offset
        self.recording = False

    def read_range_and_beacon_id(self) -> float:
        indexInfo = self.iirx.read_next_packet()
        tmsp_ping_rx = indexInfo.pre_idx * self.buf_len + indexInfo.buf_idx
        dt = tmsp_ping_rx/self.sample_freq
        distance = 0.5 * dt * self.sos - self.offset
        data = indexInfo.peak_val
        return distance, indexInfo.pre_idx, data
        #return indexInfo.buf_idx, indexInfo.pre_idx, data

    def debug_read(self) -> float:
        indexInfo = self.iirx.read_next_packet()
        return indexInfo.buf_idx, indexInfo.pre_idx, indexInfo.peak_val
    
    def pipe_to_file(self, filepath: str, num_measurements):
        f = open(filepath, "w")
        f.write("range\n")

        for i in range(num_measurements):
            if i%10==0: print("Reading Packet", i, "of", num_measurements)
            d = self.read_range()
            if d >= 0:
                f.write(str(d)+"\n")

        f.close()

    def stop_callback(self, key):
        self.recording = False
    
    def calibrate(filepath_in:str, filepath_out:str, offset:float):
        '''Offset is subtracted from each range measurement.'''
        df = pd.read_csv(filepath_in)

        f = open(filepath_out, "w")
        f.write("range\n")

        for d in df["range"]:
            f.write(str(d-offset)+"\n")
        f.close()


    
    def plot_from_file(filepath: str, show=True):
        print("Plotting from file...")
        df = pd.read_csv(filepath)
        plt.figure()
        plt.plot(df["range"])
        plt.xlabel("measurement no.")
        plt.ylabel("Range (m)")
        plt.title(filepath)

        if (show):
            plt.show()
'''
    def pipe_to_file_start_stop(self, filepath: str):
        self.recording= True
        print("Starting recording. Press enter to stop")
        listener = Listener(on_press = self.stop_callback)   
        listener.start()
        f = open(filepath, "w")
        f.write("range\n")
        i=0
        while self.recording:
            if i%100==0: print("Reading Packet", i)
            d = self.read_range()
            if d >= 0:
                f.write(str(d)+"\n")
            i+=1
 
        f.close()
        print("Recording ended. File saved to ", filepath)
'''


if __name__=="__main__":
    rc = RangeCalc("COM3", 115200, 10, 30.012)

    #for i in range(0,1000):
    #    print(rc.read_range())
    #quit()

    file = "./python/range_data/set.csv"
    #rc.pipe_to_file(file, 2500)
    #RangeCalc.plot_from_file(file)

    #rc.pipe_to_file_start_stop(file)
    #RangeCalc.plot_from_file(file)


    #file = "./python/range_data/2100_test_2.csv"
    #RangeCalc.plot_from_file(file)
    

    

    
    








        