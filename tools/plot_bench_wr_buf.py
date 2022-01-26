import matplotlib.pyplot as plt
import numpy as np
import os
import pandas as pd

this_file_dir = os.path.abspath(os.path.dirname(__file__))

mks= ["o", "v", "^", "<", ">", "s", "p", "*", "D", "P"]
lineType = ["-", "--",":", "-." ]

data_path = os.path.join(this_file_dir, "..", "output", "micro_bench", "microbench_wr_buf.csv")

df = pd.read_csv(data_path)

xAxis= df["wss"].to_numpy()/1024
wr_100= df[" 4/4XPline WA"].to_numpy()
wr_75 = df[" 3/4XPline WA"].to_numpy()
wr_50 = df[" 2/4XPline WA"].to_numpy()
wr_25 = df[" 1/4XPline WA"].to_numpy()




plt.figure(figsize = (6,3.6))

plt.grid()
plt.plot(xAxis, wr_100, "o"+lineType[0],label = "100% Write",markevery=7, markerfacecolor='none', color = "black", markersize = 8, linewidth=2)
plt.plot(xAxis, wr_75,"^"+lineType[1], label = "75%Write",markevery=7, markerfacecolor='none', color = "black", markersize = 8, linewidth=2)
plt.plot(xAxis, wr_50,"s"+lineType[1], label = "50%Write",markevery=7, markerfacecolor='none', color = "black", markersize = 8, linewidth=2)
plt.plot(xAxis, wr_25,"x"+lineType[1], label = "25%Write",markevery=7, markerfacecolor='none', color = "black", markersize = 8, linewidth=2)

plt.xticks(np.arange(0, max(xAxis)+1, 4), fontsize=14)
plt.yticks(np.arange(0, 4.0, 0.5), fontsize=14)
plt.ylabel("Write amplification", fontsize=14)
plt.xlabel("Working set size (KB)", fontsize=14)

plt.legend(loc="upper left", ncol =2, fontsize=14, bbox_to_anchor=(0, 1.3))
foo_fig = plt.gcf() # 'get current figure'
foo_fig.savefig(os.path.join(this_file_dir, "..", "output", "micro_bench", "write_buf.png"),bbox_inches='tight', format='png', dpi=1000,pad_inches=0.0)


plt.figure(figsize = (6,3.6))


ones = np.ones(xAxis.shape)
plt.grid(linestyle='--', axis= 'y')
plt.plot(xAxis, ones - wr_25/4, lineType[0],label = "Write buffer hit ratio",markevery=6, markerfacecolor='none', color = "black", markersize = 10, linewidth=2)

plt.xticks(np.arange(0, max(xAxis)+1, 4), fontsize=14)
plt.yticks(np.arange(0, 1.5, 0.2), fontsize=14)
plt.ylabel("Buffer hit ratio", fontsize=14)
plt.xlabel("Working set size (KB)", fontsize=14)

plt.legend(loc="upper left", ncol =2, fontsize=14)
foo_fig = plt.gcf() # 'get current figure'
foo_fig.savefig(os.path.join(this_file_dir, "..", "output", "micro_bench", "write_buf_hit_ratio.png"),bbox_inches='tight', format='png', dpi=1000,pad_inches=0.0)