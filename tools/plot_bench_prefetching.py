import numpy as np
import os
import matplotlib.pyplot as plt
import pandas as pd

this_file_dir = os.path.abspath(os.path.dirname(__file__))


mks= ["o", "v", "s", "<", ">", "^", "p", "*", "D", "P"]
lineType = ["-", "--",":", "-." ]

data_path = os.path.join(this_file_dir, "..", "output", "micro_bench", "microbench_trigger_prefetching.csv")

df = pd.read_csv(data_path)

wss = df["wss"].to_numpy()

theoretical_read= df[" 256 ideal written data"].to_numpy()
imc_read_256B= df[" 256 imc read"].to_numpy()
media_read_256B= df[" 256 media read"].to_numpy()

imc_read_512B= df[" 512 imc read"].to_numpy()
media_read_512B= df[" 512 media read"].to_numpy()

imc_read_1KB= df[" 1024 imc read"].to_numpy()
media_read_1KB= df[" 1024 media read"].to_numpy()

# imc_read_2KB= textdata[:,8]
# media_read_2KB= textdata[:,9]

# imc_read_4KB= textdata[:,10]
# media_read_4KB= textdata[:,11]


fig, ax1 = plt.subplots(figsize = (8,4.96))
plt.grid()
plot0=ax1.plot(wss,media_read_256B/theoretical_read,mks[0]+lineType[0], label = "256B_PM", markerfacecolor='none', color = "black", markersize = 10, linewidth=2)
plot2=ax1.plot(wss,media_read_512B/theoretical_read,mks[1]+lineType[0], label = "512B_PM", markerfacecolor='none', color = "black", markersize = 10, linewidth=2)
plot4=ax1.plot(wss,media_read_1KB/theoretical_read,mks[2]+lineType[0], label = "1KB_PM", markerfacecolor='none', color = "black", markersize = 10, linewidth=2)


plot1=ax1.plot(wss,imc_read_256B/theoretical_read,mks[0]+lineType[1], label = "256B_iMC", markerfacecolor='none', color = "black", markersize = 10, linewidth=2)
plot3=ax1.plot(wss,imc_read_512B/theoretical_read,mks[1]+lineType[1], label = "512B_iMC", markerfacecolor='none', color = "black", markersize = 10, linewidth=2)
plot5=ax1.plot(wss,imc_read_1KB/theoretical_read,mks[2]+lineType[1], label = "1KB_iMC", markerfacecolor='none', color = "black", markersize = 10, linewidth=2)

ax1.set_xscale("log", base=2)
xticks=np.logspace(10,31,12,base=2)


ax1.legend(loc="upper right", fontsize = 16, ncol=2)

ax1.set_ylabel('Read Ratio', fontsize = 16)
ax1.set_xlabel('Working set size (bytes)', fontsize = 16)
ax1.grid(linestyle='--')
ax1.tick_params(axis='y', labelsize=16) 
ax1.tick_params(axis='x', labelsize=16)

foo_fig = plt.gcf()  # 'get current figure'
foo_fig.savefig(os.path.join(this_file_dir, "..", "output", "micro_bench", "prefetching.png"),
                bbox_inches='tight', format='png', dpi=1000, pad_inches=0.0)
