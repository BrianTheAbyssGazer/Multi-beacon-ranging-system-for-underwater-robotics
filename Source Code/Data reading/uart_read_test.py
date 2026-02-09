import serial
import struct
import threading
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Button,RadioButtons
from collections import deque
from matplotlib.ticker import FuncFormatter

MAX_SAMPLES = 1000
PORT="COM5"
data_buffer = deque([0] * MAX_SAMPLES, maxlen=MAX_SAMPLES)
COMMANDS = {
    "Frequency 1" : bytes([0xAA if i<4 else 0xCC for i in range(8)]),
    "Frequency 2" : bytes([0xCC if i<4 else 0xF0 for i in range(8)]),
    "Frequency 3" : bytes([0xF0 if i<4 else 0xF0 for i in range(8)]),
    "Frequency 4" : bytes([0xFF if i<4 else 0x00 for i in range(8)]),
    "pwm 1" : bytes([0xCC if i<4 else 0xEE for i in range(8)]),
    "pwm 2" : bytes([0xCC if i<4 else 0x88 for i in range(8)]),
    "Phase 1" : bytes([0xAA if i <4 else 0x55 for i in range(8)]),
    "Phase 2" : bytes([0x55 if i <4 else 0xAA for i in range(8)]),
    "Phase 3" : bytes([0xCC if i <4 else 0x33 for i in range(8)])
}
selected_command="Frequency 1"


signal=400
serial_lock = threading.Lock() # Prevents simultaneous read/write collisions

def rx_data(ser, data_buffer, signal,lock):
    i=0
    while True:
        with lock:
            if ser.in_waiting >= 3:
                flag = ser.read(1)
                #print(flag)
                if flag==b'\xf0':
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

def setup_plot():
    ser = serial.Serial(PORT, 115200,timeout=0.1)

    thread = threading.Thread(target=rx_data, args=(ser, data_buffer, signal,serial_lock), daemon=True)
    thread.start()

    def scale_x(x, pos):
        return f'{x * 0.72:.2f}'
    
        # --- Plot Setup ---
    fig, ax = plt.subplots(figsize=(20, 5))
    plt.subplots_adjust(left=0.02,right=0.9,bottom=0.05,top=1) # Make room for the button
    #ax.xaxis.set_major_formatter(FuncFormatter(scale_x))
    line, = ax.plot(data_buffer)
    ax.set_ylim(0, 14000) # Adjust based on your expected int range
    #ax.set_ylim(86, 94) # Adjust based on your expected int range

    ax_radio = plt.axes([0.9, 0.1, 0.1, 0.9], facecolor='#f0f0f0')
    radio = RadioButtons(ax_radio, list(COMMANDS.keys()))
    
    def select_cmd(label):
        global selected_command
        selected_command = label
    radio.on_clicked(select_cmd)
    
    ax_btn = plt.axes([0.9, 0, 0.1, 0.1])
    btn = Button(ax_btn, 'Transmit Selected', color='lightgreen')
    
    def send_data(event):
        print(selected_command)

        cmd_name = selected_command
        data_to_send = COMMANDS[cmd_name]
        with serial_lock:
            print(f"Sending: {data_to_send.hex(' ')}")
            ser.write(data_to_send)

    btn.on_clicked(send_data)

    def update(frame):
        # Update the plot line data
        line.set_ydata(data_buffer)
        return line,
    ani = FuncAnimation(fig, update, interval=30, blit=True, cache_frame_data=False)
    plt.show()

if __name__ == "__main__":
    setup_plot()