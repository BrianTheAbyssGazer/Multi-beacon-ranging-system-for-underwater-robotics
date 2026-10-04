import serial
import os
import time
import csv
import matplotlib.pyplot as plt

os.chdir(os.path.dirname(os.path.abspath(__file__)))

MAX_SAMPLES = 5700
PORT = "COM8"
TEST_TYPE = "box full sample"

def rx_data(ser, max_samples):
    t1 = []
    t2 = []
    t3 = []
    a1 = []
    a2 = []
    a3 = []
    samples_collected = 0
    
    print(f"Starting data collection... Waiting for {max_samples} samples.")
    start_time = time.perf_counter()

    try:
        while samples_collected < max_samples:
            if ser.in_waiting >= 7: # 1 reading consists of 7 bytes
                flag = ser.read(1) # first byte is a flag
                if flag == b'\xf3':
                    payload = ser.read(6)
                    
                    # Parse the data
                    char1 = chr(payload[0])
                    char2 = chr(payload[1])
                    amplitude_byte = payload[2:4]
                    gain_byte = payload[4:6]
                    
                    amplitude = int.from_bytes(amplitude_byte, byteorder='little')
                    gain = int.from_bytes(gain_byte, byteorder='little')
                    
                    # Record time and amplitude
                    current_time = time.perf_counter() - start_time
                    
                    samples_collected += 1
                    
                    # Print occasionally to monitor progress without overwhelming the console
                    #if samples_collected % 100 == 0 or samples_collected == max_samples:
                    print(f"ID: {char1}{char2} | Amp: {amplitude} | gain: {gain}")
                    if char1=='A':
                        if char2=='1':
                            a1.append(amplitude)
                            t1.append(current_time)
                        elif char2=='2':
                            a2.append(amplitude)
                            t2.append(current_time)
                        elif char2=='3':
                            a3.append(amplitude)
                            t3.append(current_time)
                        
    except KeyboardInterrupt:
        # Allows you to press Ctrl+C to stop early and still plot the data you managed to collect
        print("\nCollection stopped early by user.")

    return t1,t2,t3, a1,a2,a3

if __name__ == "__main__":
    ser = serial.Serial(PORT, 115200, timeout=0.1)
    
    try:
        t1,t2,t3, a1,a2,a3 = rx_data(ser, MAX_SAMPLES)
    finally:
        ser.close() 
        print("Serial port closed.")

    # Only proceed if we actually collected data
    if a1 or a2 or a3:
        # 1. Save to CSV
        #csv_filename = f"{TEST_TYPE}_data.csv"
        #with open(csv_filename, 'w', newline='') as f:
        #    writer = csv.writer(f)
        #    writer.writerow(['Time (s)', 'Amplitude'])
        #    for t, a in zip(times, amplitudes):
        #        writer.writerow([t, a])
        #print(f"Data successfully saved to {csv_filename}")

        # 2. Plot the data
        plt.figure(figsize=(10, 6))
        plt.plot(t1, a1, label='M1', color='r', marker='.', linestyle='-', markersize=2, linewidth=0.8)
        plt.plot(t2, a2, label='M2', color='g', marker='.', linestyle='-', markersize=2, linewidth=0.8)
        plt.plot(t3, a3, label='M3', color='b', marker='.', linestyle='-', markersize=2, linewidth=0.8)
        plt.xlabel('Time (seconds)')
        plt.ylabel('Amplitude')
        plt.title(f'Amplitude vs Time ({TEST_TYPE})')
        plt.grid(True, linestyle='--', alpha=0.7)
        plt.legend()
        plt.tight_layout()
        
        print("Displaying plot... Close the window to exit the script.")
        plt.show()
    else:
        print("No data was collected.")