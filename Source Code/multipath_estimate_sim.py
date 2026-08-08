import numpy as np
import matplotlib.pyplot as plt

# 1. Generate a standard 31-chip Gold Code
def generate_gold_code():
    # LFSR polynomials for preferred pairs
    reg1 = np.array([1, 1, 0, 0, 1]) # x^5 + x^2 + 1
    reg2 = np.array([1, 0, 1, 1, 1]) # x^5 + x^4 + x^3 + x^2 + 1
    code = []
    for _ in range(16):
        code.append(1 if (reg1[-1] ^ reg2[-1]) == 1 else -1)
        fb1 = reg1[2] ^ reg1[4]
        reg1 = np.roll(reg1, 1); reg1[0] = fb1
        fb2 = reg2[1] ^ reg2[2] ^ reg2[3] ^ reg2[4]
        reg2 = np.roll(reg2, 1); reg2[0] = fb2
    return np.array(code)

# 2. Setup Data and Signal
data_bits = np.array([1, 0, 1, 1, 0, 1, 0, 0])
data_polar = np.where(data_bits == 1, 1, -1)
gold_code = generate_gold_code()
samples_per_chip = 1
chip_seq = np.repeat(gold_code, samples_per_chip)

# 3. DSSS Encoding: Spread each bit by the Gold code
encoded_signal = np.concatenate([bit * chip_seq for bit in data_polar])

# 4. Add Multipath Interference (Path 1 + Path 2)
path1 = encoded_signal
path2 = np.zeros_like(path1)
delay = samples_per_chip * 2 # 2 chip delay
path2[delay:] = encoded_signal[:-delay] # 60% amplitude echo
rx_signal = path1*1.1 + path2*0.9 + path2*0.3 + path2*0.1
rx_signal = np.where(rx_signal >1, rx_signal-1, rx_signal)
rx_signal = np.where(rx_signal <-1, rx_signal+1, rx_signal)

# 5. Correlation Detection
# We slide the template across the stream. Peaks indicate bit starts.
correlation = np.convolve(rx_signal, chip_seq[::-1], mode='valid')

# 6. Plotting Results
plt.figure(figsize=(12, 6))
plt.subplot(2, 1, 1)
plt.plot(rx_signal, label='Received Signal (8 Bits + Multipath)')
plt.title('DSSS Encoded 8-bit Data Stream')
plt.legend(loc='upper right')

plt.subplot(2, 1, 2)
plt.plot(correlation, color='darkorange', label='Correlation Output')
plt.title('Correlation Peaks: Upward = "1", Downward = "0"')
# Show where the bits start
for i in range(8):
    plt.axvline(x=i * len(chip_seq), color='gray', linestyle='--', alpha=0.4)
plt.tight_layout()
plt.show()