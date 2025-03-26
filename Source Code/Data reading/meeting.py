from uart_reader import UARTReader
from index_info_rx import IndexInfoRX






#file = "max_static_1.csv"
file = "max_dynamic_2.csv"
IndexInfoRX.plot_val_from_file(file, show=False)
IndexInfoRX.plot_diffs_from_file(file)

