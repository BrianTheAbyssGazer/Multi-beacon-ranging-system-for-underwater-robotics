from range import RangeCalc


#rc = RangeCalc("COM4", 115200, 10, 30.012)

#for i in range(0,1000):
#    print(rc.read_range())
#quit()

file = "./python/range_exp/exp_5.csv"
#rc.pipe_to_file(file, 2500)
#RangeCalc.plot_from_file(file)

#rc.pipe_to_file_start_stop(file)
#RangeCalc.plot_from_file(file, show=False)


#file = "./python/range_data/2100_test_2.csv"
#RangeCalc.plot_from_file(file)


# FFT noise stuff
file = "./python/range_exp/exp_5.csv"

import numpy as np
from numpy.fft import rfft
from numpy.fft import fftfreq
from matplotlib import pyplot as plt


def fft_plot(signal : np.ndarray, sample_freq, show=True):
        fft = rfft(signal)
        fft_mag = np.abs(fft)

        N = fft_mag.shape[0] #number of fft bins

        bin_width = sample_freq /(2*N) # factor of 2 is because only half (0-pi) of the fft is computed (rest is mirror)
        fft_mag[0] = 0.0
        fft_mag[1] = 0.0

        plt.figure()
        plt.plot(bin_width*np.array([i for i in range(N)]), fft_mag)
        plt.xlabel("Frequency (Samples^{-1})")
        plt.ylabel("Frequency intensity")
        plt.title("FFT mag")

        if show:
            plt.show()


import pandas as pd
df = pd.read_csv(file)
noise_cha = np.array(df["range"])[170:-1]

#fft_plot(noise_cha, 1/(130e-3), show=False) # TODO check sample freq
plt.figure()
plt.plot(noise_cha)

fft_plot(noise_cha, 1, show=False) # TODO check sample freq

# low pass filter
n = 20
kernel = np.ones((n))/n
filt = np.convolve(noise_cha, kernel, mode='valid')

plt.figure()
plt.plot(noise_cha)
plt.plot([i+n//2 for i in range(filt.shape[0])],filt)

plt.show()




        