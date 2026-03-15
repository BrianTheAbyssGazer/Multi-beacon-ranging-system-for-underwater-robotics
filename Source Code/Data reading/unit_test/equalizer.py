import numpy as np
import matplotlib.pyplot as plt
import os
os.chdir(os.path.dirname(os.path.abspath(__file__)))

filename = "box_code.npy"
rx_signal = np.load(filename).astype(float)
rx_signal-=np.mean(rx_signal)
# --- Configuration ---
fc = 115000          # Carrier Frequency (115 kHz)
fs = 6 * fc          # Sampling Frequency (6x Carrier)
cycles_per_sym = 50  # Cycles in one symbol
samples_per_sym = int(fs * (cycles_per_sym / fc)) # 240 samples

num_preamble = 8
num_data = 24
total_symbols = num_preamble + num_data

# --- Data Generation ---
# 8 Preamble (1s) + 32 Data bits (Random)
preamble_bits = np.array([1,1,1,0,0,1,0,1])

# --- Signal Generation ---
t = np.arange(len(rx_signal)) / fs
carrier = np.cos(2 * np.pi * fc * t)

# Summing paths + Noise
#rx_signal = path1 + path2 + path3


from scipy.fft import fft, ifft

# --- 1. Create the Preamble Template ---
# The template is just the first 8 symbols (8 * 240 samples)
# We use the 'encoded_bits' and 'carrier' logic from the previous step
template_encoded = 2 * preamble_bits - 1
t_template = np.arange(num_preamble * samples_per_sym) / fs
template_carrier = np.cos(2 * np.pi * fc * t_template)
template_signal = np.repeat(template_encoded, samples_per_sym) * template_carrier

# --- 2. FFT Correlation ---
# To avoid circular convolution wrap-around, we pad to the next power of 2
n_fft = len(rx_signal) + len(template_signal) - 1

# Transform both to frequency domain
rx_fft = fft(rx_signal, n=n_fft)
template_fft = fft(template_signal, n=n_fft)

# Perform correlation: Multiply RX by the Conjugate of the Template
correlation_fft = rx_fft * np.conj(template_fft)

# Inverse FFT to get back to time domain
correlation_time = np.real(ifft(correlation_fft))
filter=np.array([1,1,1,1,1,1])
filter2=np.array([1,1,1])
phase = np.correlate(np.multiply(rx_signal,carrier),filter,'same')
# --- 3. Find the Peak ---
# The peak index tells us where the preamble starts
peak_idx = np.argmax(correlation_time)
print(f"Preamble detected at sample index: {peak_idx}")
power=np.correlate(abs(correlation_time),filter,'same')
power=np.correlate(power,filter2,'same')
#----------------peak detection-----------------
n_peaks=3
peaks=np.zeros(n_peaks)
ipeaks=np.zeros(n_peaks,'int')
for i in range(1,len(power)-1):
    if power[i-1]<power[i] and power[i+1]<power[i]:
        if power[i]>min(peaks):
            j_min=np.argmin(peaks)
            peaks[j_min]=power[i]
            ipeaks[j_min]=i
#----------------equalization-------------
for i in range(len(rx_signal)):
    for j in range(n_peaks):
        if i>=ipeaks[j] and i<ipeaks[j]+len(template_signal):
            rx_signal[i]-=rx_signal[i-ipeaks[j]]*0.5

phase1 = np.correlate(np.multiply(rx_signal,carrier),filter,'same')

# --- 4. Visualization ---
plt.figure(figsize=(12, 6))

plt.subplot(2, 1, 1)
i_trim=int(len(template_signal)/2)
plt.plot(power[:-len(template_signal)], color='red')
for i in range(3):
    plt.axvline(x=ipeaks[i], color='black', linestyle='--')
plt.legend()
plt.grid(True)

plt.subplot(2, 1, 2)
plt.plot(phase, color='green')
plt.plot(phase1, color='blue')
for i in range(3):
    plt.axvline(x=ipeaks[i], color='black', linestyle='--')
plt.legend()
plt.grid(True)




plt.tight_layout()
plt.show()