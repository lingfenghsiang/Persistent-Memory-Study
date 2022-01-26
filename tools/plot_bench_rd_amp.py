import matplotlib.pyplot as plt
import numpy as np
import matplotlib.ticker as ticker
import pandas as pd
import os

this_file_dir = os.path.abspath(os.path.dirname(__file__))

mks= ["o", "v", "^", "<", ">", "s", "p", "*", "D", "P"]
lineType = ["-", "--",":", "-." ]

data_path = os.path.join(this_file_dir, "..", "output", "micro_bench", "microbench_rd_amp.csv")

df = pd.read_csv(data_path)

xAxis= df["wss"].to_numpy()
line0= df[" 1 line RA"].to_numpy()
line1= df[" 2 line RA"].to_numpy()
line2= df[" 3 line RA"].to_numpy()
line3= df[" 4 line RA"].to_numpy()

print(line0)

fig, ax1 = plt.subplots(figsize = (8,4.96))
plt.grid()
plot0=ax1.plot(xAxis/1024,line0,"o"+lineType[0], label = "read 4 cachelines",markevery=3, markerfacecolor='none', color = "black", markersize = 10, linewidth=2)
plot1=ax1.plot(xAxis/1024,line1,"s"+lineType[0], label = "read 3 cachline",markevery=3, markerfacecolor='none', color = "black", markersize = 10, linewidth=2)
plot1=ax1.plot(xAxis/1024,line2,"^"+lineType[0], label = "read 2 cachline",markevery=3, markerfacecolor='none', color = "black", markersize = 10, linewidth=2)
plot1=ax1.plot(xAxis/1024,line3,"x"+lineType[0], label = "read 1 cachline",markevery=3, markerfacecolor='none', color = "black", markersize = 10, linewidth=2)

# ax1.set_xscale("log", base=2)
# xticks=np.logspace(12,32,11,base=2)
# ax1.set_xticks(xticks)
ax1.set_ylim(0,5)
# ax2.set_ylim(-1.5,7)

ax1.legend(loc="upper right", fontsize = 15, ncol=2)
xtics= np.arange(0,34,2)
ytics= np.arange(0,7,0.5)
plt.xticks(xtics, fontsize = 16)
plt.yticks(ytics, fontsize = 16)
ax1.set_ylabel('Read amplification', fontsize = 15)
ax1.set_xlabel('Working set size(KB)', fontsize = 15)
# ax2.set_ylabel('write back ratio (media write/memory controller write)')

foo_fig = plt.gcf() # 'get current figure'
foo_fig.savefig(os.path.join(this_file_dir, "..", "output", "micro_bench", "read_amp.png"),bbox_inches='tight', format='png', dpi=1000,pad_inches=0.0)
