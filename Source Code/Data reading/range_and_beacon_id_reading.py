from range import RangeCalc
import numpy as np
from numpy.fft import rfft
from numpy.fft import fftfreq
from matplotlib import pyplot as plt
import pandas as pd

rc = RangeCalc("COM3", 115200, 10, 30.012)

# Initialize live plot
plt.ion()
fig, ax = plt.subplots()
distance1, index1, distance2, index2, distance3, index3 = [], [], [], [], [], []

line1, = ax.plot(index1, distance1, label="beacon 1 distance")
line2, = ax.plot(index2, distance2, label="beacon 2 distance")
line3, = ax.plot(index3, distance3, label="beacon 3 distance")

ax.set_xlabel("Sample Index")
ax.set_ylabel("distance (cm)")
ax.set_title("Multi-beacon distance")
ax.legend()
ax.grid()
i,i1,i2,i3=0,0,0,0
while i <100:  
    distance, pfx, data = rc.read_range_and_beacon_id()
    i+=1
    print(f"distance={distance}m, pfx={pfx}, data={data}")  # Print the filtered value
    
    # Update plot data
    if data==1:
        distance1.append(distance)
        i1+=1
        index1.append(i1)
    elif data==2:
        distance2.append(distance)
        i2+=1
        index2.append(i2)
    elif data==3:
        distance3.append(distance)
        i3+=1
        index3.append(i3)

    line1.set_xdata(index1)
    line1.set_ydata(distance1)
    line2.set_xdata(index2)
    line2.set_ydata(distance2)
    line3.set_xdata(index3)
    line3.set_ydata(distance3)
    ax.relim()
    ax.autoscale_view()
    plt.draw()
    # plt.pause(0.01)  # Adjust for smoother updates

plt.ioff()
plt.show()
np.savez('distances.npz', arr_1=np.array(distance1), arr_2=np.array(distance2), arr_3=np.array(distance3))