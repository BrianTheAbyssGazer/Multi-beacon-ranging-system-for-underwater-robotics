from uart_reader import UARTReader


UARTReader.load_and_plot_fancy("./python/data/230Hz_0pdc8_train_water.csv", 1.2e6, "Signal in fish-tank, no acoustic foam", show=False)

UARTReader.load_and_plot_fancy("./python/data/acc_foam_2.csv", 1.2e6, "Signal in fish-tank, with acoustic foam", show=False)

UARTReader.load_and_plot_fancy("./python/data/acc_foam_2.csv", 1.2e6,  "Signal in fish-tank, with acoustic foam, one period", w_start=1090, w_end=7690)
