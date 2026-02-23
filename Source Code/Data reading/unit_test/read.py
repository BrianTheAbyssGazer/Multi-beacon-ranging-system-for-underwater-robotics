import numpy as np
import os
import matplotlib.pyplot as plt
from matplotlib.widgets import Button
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
def run_multi_viewer():
    # Configuration
    types = ["digital", "air", "water"]
    colors = {"digital": "blue", "air": "green", "water": "red"}
    indices = range(1, 10) # 1 to 9
    
    fig, ax = plt.subplots(figsize=(18, 5))
    plt.subplots_adjust(left=0.04,right=0.99,bottom=0.1,top=1) # Make room for the button
    
    # Create 3 line objects (one for each type)
    lines = {}
    for t in types:
        line, = ax.plot([], [], label=t, color=colors[t], alpha=0.8)
        lines[t] = line

    ax.legend(loc='upper right')
    ax.grid(True, linestyle='--', alpha=0.6)

    def load_group(index):
        all_y_data = []
        found_any = False
        
        for t in types:
            filename = f"{t}_{index}.npy"
            if os.path.exists(filename):
                y_data = np.load(filename)
                x_data = np.arange(len(y_data))*1.45
                
                lines[t].set_data(x_data, y_data)
                all_y_data.extend(y_data)
                found_any = True
            else:
                # If file missing, hide the line
                lines[t].set_data([], [])
                print(f"Warning: {filename} not found.")

        if found_any:
            # Rescale the view to fit the new combined data
            ax.set_xlim(0, 2000 *1.45) # Assuming 1000 samples
            if all_y_data:
                ax.set_ylim(min(all_y_data) * 0.9, max(all_y_data) * 1.1)
            ax.set_title(f"Viewing Data Group: Index {index}")
            plt.draw()

    # --- Create Selector Buttons (1-9) ---
    btn_objs = []
    width = 0.09
    for i,type in enumerate(signal_type):
        ax_btn = plt.axes([0.01 + (i*0.11), 0.01, width, 0.04])
        btn = Button(ax_btn, str(type), color='lightgrey', hovercolor='skyblue')
        # Lambda captures current i
        btn.on_clicked(lambda event, idx=type: load_group(idx))
        btn_objs.append(btn)

    # Load index 1 by default
    load_group(signal_type[0])

    plt.show()

if __name__ == "__main__":
    run_multi_viewer()