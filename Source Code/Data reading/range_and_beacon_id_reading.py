from range import RangeCalc
import numpy as np
from numpy.fft import rfft
from numpy.fft import fftfreq
from matplotlib import pyplot as plt
import pandas as pd

rc = RangeCalc("COM3", 115200, 20, 30.012)

# Initialize live plot
# plt.ion()
fig, ax = plt.subplots()
x_data, y_data = [], []
line, = ax.plot(x_data, y_data, label="Filtered Data (0-1)")
ax.set_xlabel("Sample Index")
ax.set_ylabel("Range Value")
ax.set_title("Real-Time Filtered Range Data (0-1)")
ax.legend()
ax.grid()

for i in range(0, 1000):  
    value, detected_beacon_id = rc.read_range_and_beacon_id()
    
    if 0 < value < 1:  # Filter values between 0 and 1
        print(f"range={value}, detected beacon id={detected_beacon_id}")  # Print the filtered value
        
        # Update plot data
        x_data.append(i)
        y_data.append(value)

        line.set_xdata(x_data)
        line.set_ydata(y_data)
        ax.relim()
        ax.autoscale_view()
    # plt.draw()
    # plt.pause(0.01)  # Adjust for smoother updates



# plt.ioff()
# plt.show()
