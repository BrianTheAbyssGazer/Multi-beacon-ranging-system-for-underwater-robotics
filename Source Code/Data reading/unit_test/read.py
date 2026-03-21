import numpy as np
import os
from enum import Enum
import matplotlib.pyplot as plt
os.chdir(os.path.dirname(os.path.abspath(__file__)))

filename = "air_BPSK.npy"
rx_signal = np.load(filename).astype(float)[100:]
f_sonar=115000
N_sample=12
N_CYCLE=8
N_data=4
fs = f_sonar*N_sample        # Sampling frequency (Hz)
N_plot = len(rx_signal)
T = float(N_plot)/fs         # Duration (seconds)
t = np.linspace(0, T, N_plot)
N_plot = len(rx_signal)
clock=np.sin(f_sonar*2*np.pi*t-1.2)

sin_lut= [0.0, 0.19509, 0.382683, 0.55557, 0.707107, 0.83147, 0.92388, 0.980785, 1.0, 0.980785, 0.92388, 0.83147,
		0.707107, 0.55557, 0.382683, 0.19509, 0.0, -0.19509, -0.382683, -0.55557, -0.707107, -0.83147, -0.92388, -0.980785, -1.0, -0.980785,
		-0.92388, -0.83147, -0.707107, -0.55557, -0.382683, -0.19509]
cos_lut= [1.0, 0.980785, 0.92388, 0.83147, 0.707107, 0.55557, 0.382683, 0.19509, 0.0, -0.19509, -0.382683, -0.55557,
		-0.707107, -0.83147, -0.92388, -0.980785, -1.0, -0.980785, -0.92388, -0.83147, -0.707107, -0.55557, -0.382683, -0.19509, -0.0, 0.19509,
		0.382683, 0.55557, 0.707107, 0.83147, 0.92388, 0.980785]

class MPDSearchState(Enum):
    NO_SIGNAL = 0
    PREAMBLE = 1
    YES_SIGNAL = 2
DEAD_INTERVAL = 256    
TWOPI = 2 * np.pi
def demodulation(rx_signal):
    rx_data=np.zeros(N_data)
    N_plot = len(rx_signal)
    corr=np.zeros(N_plot)
    q_c=np.zeros(N_plot)
    i_c=np.zeros(N_plot)
    p=np.zeros(N_plot)

    symbol_counter=0
    sample_counter=0
    phase=0
    phase_int=0
    corr_sum=0
    cur_idx=0
    search_sub_state=MPDSearchState.NO_SIGNAL
    while cur_idx<N_plot:
        cur_val=rx_signal[cur_idx]
        if search_sub_state == MPDSearchState.NO_SIGNAL:
            if cur_val > 2084 or cur_val < 1684:
            #if cur_val>0:
                print(cur_idx,rx_signal[cur_idx])
                sample_counter = 0
                symbol_counter = 0
                search_sub_state = MPDSearchState.YES_SIGNAL
            else:
                cur_idx += 2

        elif search_sub_state == MPDSearchState.YES_SIGNAL:
            if symbol_counter < N_data:
                if sample_counter < (12 * N_CYCLE):
                    # Handle buffer masking and indexing
                    pre_val = rx_signal[cur_idx - 3]
                    
                    # Normalizing and conversion
                    imag = (float(pre_val) - 1884) * 0.000488281
                    real = (float(cur_val) - 1884) * 0.000488281
                    
                    # arm_sin_cos_f32 equivalent
                    rad = phase * 57.2958 * (np.pi / 180.0) # Converting to radians if phase was degrees
                    sin_val = np.sin(rad)
                    cos_val = np.cos(rad)
                    p[cur_idx]=cos_val
                    i_comp = real * cos_val + imag * sin_val
                    q_comp = imag * cos_val - real * sin_val
                    q_c[cur_idx]=q_comp
                    i_c[cur_idx]=i_comp
                    #if(i_comp>0): error=q_comp
                    #else: error=-q_comp
                    error=i_comp * q_comp
                    # Phase update with simple PLL logic
                    #current_freq += 0.0001 * error
                    phase += 0.523599 + (0.05 * error)
                    corr_sum += i_comp
                    corr[cur_idx]=i_comp
                    cur_idx += 1
                    sample_counter += 1
                else:
                    #print(cur_idx,'pre')
                    rx_data[symbol_counter] = (corr_sum > 0.0)
                    cur_idx += DEAD_INTERVAL*6-N_CYCLE*12
                    #print(cur_idx,'af')
                    sample_counter = 0
                    symbol_counter += 1
                    corr_sum = 0
            else:
                # Finalize byte
                       
                # Reset state
                symbol_counter = 0
                sample_counter = 0
                corr_sum = 0
                phase = 0
                cur_idx += N_plot
                search_sub_state = MPDSearchState.NO_SIGNAL

    return rx_data,corr,q_c,i_c,p


plt.figure(figsize=(15, 4))
plt.subplot(2,1,1)
data,corr,q_c,i_c,p=demodulation(rx_signal)
print(data)
plt.plot(rx_signal, label="rx")
#plt.plot(p*4000+1884, label="p")
plt.plot(clock*2000+1884,label='clk')

plt.plot(corr*2000+1884, label="correlation")
plt.tight_layout()
plt.legend()

plt.subplot(2,1,2)
plt.plot((q_c), label="q")
plt.plot((i_c), label="i")
plt.tight_layout()
plt.legend()
plt.show()