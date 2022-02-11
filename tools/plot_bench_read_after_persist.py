import matplotlib.pyplot as plt
import numpy as np
import os
import pandas as pd


this_file_dir = os.path.abspath(os.path.dirname(__file__))

mks= ["o", "v", "^", "<", ">", "s", "p", "*", "D", "P","x", "X"]
lineType = ["-", "--",":", "-." ]


data_path_pm = os.path.join(this_file_dir, "..", "output", "micro_bench", "microbench_read_after_persist.csv")
data_path_dram = os.path.join(this_file_dir, "..", "output", "micro_bench", "microbench_read_after_persist_dram.csv")

df_pm = pd.read_csv(data_path_pm)
df_dram = pd.read_csv(data_path_dram)
distance = df_pm["distance"].to_numpy()[0:45]
CPU_cycle_pm_mfence = df_pm[" wr_clwb_mfence RAP lat"].to_numpy()[0:45]
CPU_cycle_pm_sfence = df_pm[" wr_clwb_sfence RAP lat"].to_numpy()[0:45]
CPU_cycle_dram_mfence= df_dram[" wr_clwb_mfence RAP lat"].to_numpy()[0:45]
CPU_cycle_dram_sfence= df_dram[" wr_clwb_sfence RAP lat"].to_numpy()[0:45]
CPU_cycle_pm_nt_mfence= df_pm[" wr_nt_mfence RAP lat"].to_numpy()[0:45]
CPU_cycle_pm_nt_sfence= df_pm[" wr_nt_sfence RAP lat"].to_numpy()[0:45]

fig, ax1 = plt.subplots(figsize = (8,4.96))
plt.grid()
plot0=ax1.plot(distance,CPU_cycle_pm_mfence,mks[0]+lineType[0], label = "PM clwb + mfence",markevery=3, markerfacecolor='none', color = "black", markersize = 10, linewidth=2)
plot3=ax1.plot(distance,CPU_cycle_pm_sfence,mks[1]+lineType[1], label = "PM clwb + sfence",markevery=3, markerfacecolor='none', color = "black", markersize = 10, linewidth=2)
plot7=ax1.plot(distance,CPU_cycle_pm_nt_sfence,"D"+lineType[1], label = "PM nt-store + sfence",markevery=3, markerfacecolor='none', color = "black", markersize = 10, linewidth=2)
plot4=ax1.plot(distance,CPU_cycle_dram_sfence,"s"+lineType[0], label = "DRAM clwb + sfence",markevery=3, markerfacecolor='none', color = "black", markersize = 10, linewidth=2)
plot5=ax1.plot(distance,CPU_cycle_dram_mfence,mks[10]+lineType[1], label = "DRAM clwb + mfence",markevery=3, markerfacecolor='none', color = "black", markersize = 10, linewidth=2)

ax1.set_ylim(-200,2800)
# ax2.set_ylim(-1.5,7)

# ax1.legend(loc="upper right", fontsize = 16, ncol=2)
# xtics= np.arange(0,34,2)
ytics= np.arange(0,4000,250)
ax1.set_yticks(ytics)
plt.xticks(np.arange(0, 50, 5) , fontsize = 16)
plt.yticks( fontsize = 16)
ax1.set_ylabel('CPU cycles/iteration', fontsize = 16)
ax1.set_xlabel('Distance(cacheline)', fontsize = 16)
ax1.legend(fontsize = 16)
# ax2.set_ylabel('write back ratio (media write/memory controller write)')

foo_fig = plt.gcf() # 'get current figure'
foo_fig.savefig(os.path.join(this_file_dir, "..", "output", "micro_bench", "read_after_persist.png"),bbox_inches='tight', format='png', dpi=1000,pad_inches=0.0)