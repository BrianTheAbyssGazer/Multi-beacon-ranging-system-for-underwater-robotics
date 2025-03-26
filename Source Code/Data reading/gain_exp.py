from uart_reader import UARTReader
from matplotlib import pyplot as plt

'''
ur = UARTReader("COM6", 115200, 100000)

ur.read_frame()

ur.plot_all()

ur.save_all("gain_experiment/set.csv")
'''

#5.5m

for gi in range(2,9):
    for cp in [4,8]:
        UARTReader.load_and_plot_fancy("./python/data/gain_experiment/gi" + str(gi) + "_cp" + str(cp) +"_a.csv", 1.2e6, "RX gain index " + str(gi) + ", TX pulse width " + str(cp) + " carrier periods", show=False)

plt.show()