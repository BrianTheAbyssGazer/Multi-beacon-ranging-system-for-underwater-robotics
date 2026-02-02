import serial
import struct
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from collections import deque

MAX_SAMPLES = 10000
data_buffer = deque([0] * MAX_SAMPLES, maxlen=MAX_SAMPLES)
ser = serial.Serial("COM3", 115200,timeout=0.1)

# --- Plot Setup ---
fig, ax = plt.subplots()
line, = ax.plot(data_buffer)
ax.set_ylim(-10, 20) # Adjust based on your expected int range
ax.set_title("Real-Time Serial Data (Last 1000 Samples)")
def update(frame):
    # Read all available chunks of 4 bytes
    while ser.in_waiting >= 4:

        flag = ser.read(1)
        print(flag)
        if flag==b'\xff':
            raw_data = ser.read(4)
            print(raw_data)
            try:
                # Unpack as little-endian signed int
                value = struct.unpack('>i', raw_data)[0]
                data_buffer.append(value)
            except struct.error:
                pass

    # Update the plot line data
    line.set_ydata(data_buffer)
    return line,


ani = FuncAnimation(fig, update, interval=20, blit=True, cache_frame_data=False)

plt.show()
ser.close()
