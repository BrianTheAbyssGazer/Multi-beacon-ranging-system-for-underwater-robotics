import numpy as np
from numpy.fft import rfft
from numpy.fft import fftfreq
from matplotlib import pyplot as plt
import pandas as pd
from uart_reader import *

IN_BUF_LEN = 18e3
OUT_BUF_LEN = 3e3
CARRIER_FREQ = 115e3 #Hz
TOFM_PERIOD = 10 # buffers
r_meas_period = OUT_BUF_LEN*TOFM_PERIOD*(1/(2*CARRIER_FREQ)) #s
r_meas_freq = 1/r_meas_period

NOISE_CHARACTERIZATION = False
FISHTANK_STEP = False
LAKE_STEP = False
LAKE_PRESENTATION = False

PEAK_DETECTION = True



plt.rcParams['font.family'] = 'Times New Roman'
plt.rcParams['font.size'] = 16

####################################################################
################## Noise characterization ##########################
####################################################################
if NOISE_CHARACTERIZATION:

    print("Range Measurement Frequency:", r_meas_freq)
    file = "./python/range_exp/exp_5.csv"
    df = pd.read_csv(file)
    noise_cha = np.array(df["range"])[170:-1]

    # low pass filter
    n = 20
    kernel = np.ones((n))/n
    filt = np.convolve(noise_cha, kernel, mode='valid')

    plt.figure(figsize=(10, 6))
    plt.plot([i*r_meas_period for i in range(noise_cha.shape[0])], 
             noise_cha, color="mediumblue", label="Range Measurement")
    plt.plot([(i+n//2)*r_meas_period for i in range(filt.shape[0])], 
             filt, color="darkorange", label="Low Pass Filter")
    plt.legend()
    plt.title("Noise Chareteristics for Range Measurements at 5m")
    plt.xlabel("Time (sec)")
    plt.ylabel("Range (m)")
    plt.show()




####################################################################
########################## Fishtank Steps ##########################
####################################################################
if FISHTANK_STEP:

    print("Range Measurement Frequency:", r_meas_freq)
    file = "./python/range_data/exp_2.csv"
    df = pd.read_csv(file)
    ft_step = np.array(df["range"])

    

    plt.figure(figsize=(10, 6))
    plt.plot([i               for i in range(40)],               [0.3   for i in range(40)],               color="darkred", label="Ground Truth", ls="dashed")
    plt.plot([i*r_meas_period for i in range(ft_step.shape[0])], [0.2   for i in range(ft_step.shape[0])], color="darkred",                       ls="dashed")
    plt.plot([i*r_meas_period for i in range(ft_step.shape[0])], [0.1   for i in range(ft_step.shape[0])], color="darkred",                       ls="dashed")
    plt.plot([i*r_meas_period for i in range(ft_step.shape[0])], [0.225 for i in range(ft_step.shape[0])], color="darkred",                       ls="dashed")
    plt.plot([i*r_meas_period for i in range(ft_step.shape[0])], ft_step+0.005, color="mediumblue", label="Range Measurement")
    
    plt.legend()
    plt.title("Range Measurements in Fish Tank")
    plt.xlabel("Time (sec)")
    plt.ylabel("Range (m)")
    plt.show()



####################################################################
############################## Lake Steps ##########################
####################################################################
if LAKE_STEP:

    fig, axs = plt.subplots(nrows=3, ncols=1, figsize=(9, 10)) # THESIS

    file = "./python/range_exp/exp_1.csv"
    df = pd.read_csv(file)
    lake_step = np.array(df["range"])

    
    axs[0].plot([i*r_meas_period for i in range(lake_step.shape[0])], lake_step, color="mediumblue",)

    axs[0].set_title("0.5m Steps on Jetty", fontsize=14)
    #axs[0].set_xlabel("Time (sec)")
    axs[0].set_ylabel("Range (m)")




    file = "./python/range_exp/exp_3.csv"
    df = pd.read_csv(file)
    lake_step = np.array(df["range"])[5:-1]

    axs[1].plot([i*r_meas_period for i in range(lake_step.shape[0])], lake_step, color="mediumblue",)
    axs[1].set_title("Walk Out and Back on Jetty", fontsize=14)
    #axs[1].set_xlabel("Time (sec)")
    axs[1].set_ylabel("Range (m)")

    

    file = "./python/range_exp/exp_4.csv"
    df = pd.read_csv(file)
    lake_step = np.array(df["range"])[0:-50]

    axs[2].plot([i*r_meas_period for i in range(lake_step.shape[0])], lake_step, color="mediumblue",)
    axs[2].set_title("Walk Out to 20m", fontsize=14)
    axs[2].set_xlabel("Time (sec)")
    axs[2].set_ylabel("Range (m)")

    fig.suptitle('Range Measurements in Lake Burley Griffin', fontsize=20)

    plt.tight_layout()#rect=[0, 0, 1, 0.999])
    plt.show()




if LAKE_PRESENTATION:
    fig, axs = plt.subplots(nrows=2, ncols=1, figsize=(9, 5))   # presentation

    file = "./python/range_exp/exp_1.csv"
    df = pd.read_csv(file)
    lake_step = np.array(df["range"])

    
    axs[0].plot([i*r_meas_period for i in range(lake_step.shape[0])], lake_step, color="mediumblue",)

    axs[0].set_title("0.5m Steps on Jetty", fontsize=14)
    #axs[0].set_xlabel("Time (sec)")
    axs[0].set_ylabel("Range (m)")




    file = "./python/range_exp/exp_3.csv"
    df = pd.read_csv(file)
    lake_step = np.array(df["range"])[5:-1]

    axs[1].plot([i*r_meas_period for i in range(lake_step.shape[0])], lake_step, color="mediumblue",)
    axs[1].set_title("Walk Out and Back on Jetty", fontsize=14)
    #axs[1].set_xlabel("Time (sec)")
    axs[1].set_ylabel("Range (m)")

    

    file = "./python/range_exp/exp_4.csv"
    df = pd.read_csv(file)
    lake_step = np.array(df["range"])[0:-50]

    axs[2].plot([i*r_meas_period for i in range(lake_step.shape[0])], lake_step, color="mediumblue",)
    axs[2].set_title("Walk Out to 20m", fontsize=14)
    axs[2].set_xlabel("Time (sec)")
    axs[2].set_ylabel("Range (m)")

    fig.suptitle('Range Measurements in Lake Burley Griffin', fontsize=20)

    plt.tight_layout()#rect=[0, 0, 1, 0.999])
    plt.show()



if PEAK_DETECTION:
    
    plt.rcParams['font.size'] = 22
    UARTReader.load_and_plot_fancy("./python/data/gain_experiment/gi6_cp8_a.csv", 1.2e6, "R", w_start=int(6.4e3), w_end=int(7.2e3),y_lim=False)
    1.2e6