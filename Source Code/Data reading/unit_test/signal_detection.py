import numpy as np
import matplotlib.pyplot as plt
import os
os.chdir(os.path.dirname(os.path.abspath(__file__)))
signal_type=["1010 squarewave",
    "Frequency x2",
    "Frequency x4",
    "Frequency x8",
    "pwm low",
    "pwm high",
    "Phase shift up",
    "Phase shift down",
    "Phase+Frequency"]
plt.figure(figsize=(18, 6))
#plt.subplots_adjust(left=0.04,right=0.99,bottom=0.1,top=1) # Make room for the button
amp=7
filename = "signal_test_1010 squarewave_tank_close.npy"
y_data = np.load(filename)
y_data = (y_data - np.mean(y_data))/max(y_data)

N_plot = 2000


# reconstruct template from frequency
cycle_per_digit=20
N_digits=8
N_window=cycle_per_digit*6
timer_cycle=52./72e6
f_sonar=230769.23
x0=np.linspace(0,N_plot-1,N_plot)
N_signal=cycle_per_digit*N_digits*6
t=np.linspace(0,N_window-1,N_window)
template1=np.sin(2*np.pi*f_sonar*timer_cycle*x0[:N_window])
template0=np.sin(2*np.pi*f_sonar*timer_cycle*x0[:N_window]+np.pi)
signal_template=np.zeros(N_signal)
for i in range(8):
    if(i%2==1):
        signal_template[i*N_window:i*N_window+N_window]=np.sin(2*np.pi*f_sonar*timer_cycle*t+np.pi)
    else:
        signal_template[i*N_window:i*N_window+N_window]=np.sin(2*np.pi*f_sonar*timer_cycle*t)


# correlation 
#sig_norm = (y_data - np.mean(y_data))
#temp_norm = (sample - np.mean(y_data))
ax=plt.subplot(4,1,2)
ax.set_xlim([0,N_plot])
correlation=np.zeros(N_plot-N_signal)
for i in range(N_plot-N_signal):
    correlation[i] = np.correlate(y_data[i:i+N_signal], signal_template, mode='valid')[0]
plt.plot(x0[int(N_signal/2):-int(N_signal/2)],correlation,'orange')
phase_shift=np.argmax(correlation)

ax=plt.subplot(4,1,1)
ax.set_xlim([0,N_plot])
plt.plot(y_data[:N_plot])
plt.plot(x0[phase_shift:phase_shift+N_window*8],signal_template,'g')
ax.grid(True, linestyle='--', alpha=0.6)

ax=plt.subplot(4,1,3)
ax.set_xlim([0,N_plot])

N_c=int((N_plot-phase_shift)/N_window)
N_c=8
N_left=N_plot-phase_shift-N_c*N_window
correlation=np.zeros(N_c)
for i in range(N_c):
    correlation[i] = np.correlate(y_data[i*N_window+phase_shift:i*N_window+phase_shift+N_window], template1, mode='valid')[0]
signal_level=np.repeat(correlation,N_window)
plt.plot(x0[phase_shift:phase_shift+N_window*N_c],signal_level,'g')
amp=max(signal_level)+1
ax.set_ylim([-amp,amp])
ax=plt.subplot(4,1,4)
ax.set_xlim([0,N_plot])

for i in range(N_c):
    correlation[i] = np.correlate(y_data[i*N_window+phase_shift:i*N_window+phase_shift+N_window], template0, mode='valid')[0]
signal_level=np.repeat(correlation,N_window)
plt.plot(x0[phase_shift:phase_shift+N_window*N_c],signal_level,'orange')
amp=max(signal_level)+1
ax.set_ylim([-amp,amp])
plt.show()