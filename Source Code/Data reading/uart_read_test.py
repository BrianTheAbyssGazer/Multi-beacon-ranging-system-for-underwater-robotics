import serial
import struct
import threading
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Button,RadioButtons
from collections import deque
from matplotlib.ticker import FuncFormatter
import numpy as np
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

MAX_SAMPLES = 6200
PORT="COM7"
TEST_TYPE="air_BPSK"
data_buffer = deque([0] * MAX_SAMPLES, maxlen=MAX_SAMPLES)
COMMANDS = {
    "1010 squarewave" : bytes([0xAA if i<4 else 0xAA for i in range(8)]),
    "Frequency x2" : bytes([0xCC if i<4 else 0xCC for i in range(8)]),
    "Frequency x4" : bytes([0xF0 if i<4 else 0xF0 for i in range(8)]),
    "Frequency x8" : bytes([0xFF if i%2==0 else 0x00 for i in range(8)]),
    "pwm low" : bytes([0xCC if i<4 else 0xEE for i in range(8)]),
    "pwm high" : bytes([0xCC if i<4 else 0x88 for i in range(8)]),
    "Phase shift up" : bytes([0xAA if i <4 else 0x55 for i in range(8)]),
    "Phase shift down" : bytes([0x55 if i <4 else 0xAA for i in range(8)]),
    "Phase+Frequency" : bytes([0xCC if i <4 else 0x33 for i in range(8)])
}
selected_command="1010 squarewave"


signal=500
serial_lock = threading.Lock() # Prevents simultaneous read/write collisions

def rx_data(ser, data_buffer, save,lock):
    i=0
    while True:
        with lock:
            if save:
                save=False
            if ser.in_waiting >= 3:
                flag = ser.read(1)
                #print(flag)
                if flag==b'\xf2':
                    raw_data = ser.read(2)
                    try:
                        # Unpack as little-endian signed int
                        value = struct.unpack('<H', raw_data)[0]
                        #print(value)
                        data_buffer.append(value)
                    except struct.error:
                        pass
                elif flag==b'\xff':
                    raw_data = ser.read(4)
                    value = struct.unpack('<i', raw_data)[0]
                    print(value)
                elif flag==b'\xff':
                    raw_data = ser.read(4)
                    value = struct.unpack('<i', raw_data)[0]
                    print(value)

def setup_plot():
    ser = serial.Serial(PORT, 115200,timeout=0.1)
    global signal
    thread = threading.Thread(target=rx_data, args=(ser, data_buffer, signal ,serial_lock), daemon=True)
    thread.start()

    def scale_x(x, pos):
        return f'{x * 0.72:.2f}'
    
        # --- Plot Setup ---
    fig, ax = plt.subplots(figsize=(18, 3))
    plt.subplots_adjust(left=0.04,right=0.9,bottom=0.1,top=1) # Make room for the button
    #ax.xaxis.set_major_formatter(FuncFormatter(scale_x))
    line, = ax.plot(data_buffer)
    ax.set_ylim(0, 4500) # Adjust based on your expected int range
    #ax.set_ylim(86, 94) # Adjust based on your expected int range

    ax_radio = plt.axes([0.9, 0.1, 0.1, 0.9], facecolor='#f0f0f0')
    radio = RadioButtons(ax_radio, list(COMMANDS.keys()))
    
    def select_cmd(label):
        global selected_command
        selected_command = label
    radio.on_clicked(select_cmd)
    
    ax_btn = plt.axes([0.95, 0, 0.05, 0.1])
    btn = Button(ax_btn, 'send', color='lightgreen')
    
    def send_data(event):
        cmd_name = selected_command
        data_to_send = COMMANDS[cmd_name]
        with serial_lock:
            print(f"Sending: {data_to_send.hex(' ')}")
            ser.write(data_to_send)

    btn.on_clicked(send_data)

    def save_numpy_data(event):
        if 0:
            filename = "unit_test/"+TEST_TYPE+f"_{selected_command}_tank_close.npy"
        else:
            filename = "unit_test/"+TEST_TYPE+".npy"
        # 2. Capture and convert
        # Convert the current deque to a standard numpy array
        raw_values = np.array(list(data_buffer))
        
        try:
            # 3. Save as binary file
            np.save(filename, raw_values)
            print(f"NumPy data saved: {filename} ({raw_values.shape})")
        except Exception as e:
            print(f"NumPy save failed: {e}")

    ax_save = plt.axes([0.9, 0, 0.05, 0.1])
    btn_save = Button(ax_save, 'Save', color='lightblue', hovercolor='skyblue')
    btn_save.on_clicked(save_numpy_data)

    def update(frame):
        # Update the plot line data
        line.set_ydata(data_buffer)
        return line,
    ani = FuncAnimation(fig, update, interval=30, blit=True, cache_frame_data=False)
    plt.show()

if __name__ == "__main__":
    setup_plot()