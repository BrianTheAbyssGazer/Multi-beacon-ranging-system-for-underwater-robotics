from range import RangeCalc
import numpy as np
from numpy.fft import rfft
from numpy.fft import fftfreq
from matplotlib import pyplot as plt
import pandas as pd
import time

rc = RangeCalc("COM3", 115200, 10, 30.012)

# Initialize live plot
plt.ion()
fig, ax = plt.subplots()
distance1, distance2, distance3, t1,t2,t3 = [], [], [], [], [], []

line1, = ax.plot(t1, distance1, label="beacon 1 distance")
line2, = ax.plot(t2, distance2, label="beacon 2 distance")
line3, = ax.plot(t3, distance3, label="beacon 3 distance")

ax.set_xlabel("Sample Index")
ax.set_ylabel("distance (cm)")
ax.set_title("Multi-beacon distance")
ax.legend()
ax.grid()
i=0
start_time = time.perf_counter()
while i <100:  
    distance, pfx, data = rc.debug_read()
    i+=1
    print(f"distance={distance}m, pfx={pfx}, data={data}")  # Print the filtered value
    curren_time = time.perf_counter()
    elapsed_ms = (curren_time - start_time) * 1000
    # Update plot data
    if data==1:
        distance1.append(distance)
        t1.append(elapsed_ms)
    elif data==2:
        distance2.append(distance)
        t2.append(elapsed_ms)
    elif data==3:
        distance3.append(distance)
        t3.append(elapsed_ms)

    line1.set_xdata(t1)
    line1.set_ydata(distance1)
    line2.set_xdata(t2)
    line2.set_ydata(distance2)
    line3.set_xdata(t3)
    line3.set_ydata(distance3)
    ax.relim()
    ax.autoscale_view()
    #plt.draw()
    # plt.pause(0.01)  # Adjust for smoother updates

plt.ioff()
plt.show()
np.savez('distances.npz', d1=np.array(distance1), d2=np.array(distance2), d3=np.array(distance3), t1=np.array(t1),t2=np.array(t2),t3=np.array(t3))