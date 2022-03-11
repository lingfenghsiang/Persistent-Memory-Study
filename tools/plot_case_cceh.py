import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import os

this_file_dir = os.path.abspath(os.path.dirname(__file__))

mks= ["o", "v", "^", "<", ">", "s", "p", "*", "D", "P"]
lineType = ["-", "--",":", "-." ]

# dram
path0 = os.path.join(this_file_dir, "..", "output", "case_study", "cceh_original_dram.csv")
path1 = os.path.join(this_file_dir, "..", "output", "case_study", "cceh_with_preread_dram.csv")
# pm 
path2 = os.path.join(this_file_dir, "..", "output", "case_study", "cceh_original_pmm.csv")
path3 = os.path.join(this_file_dir, "..", "output", "case_study", "cceh_with_preread_pmm.csv")

# dram 
df0 = pd.read_csv(path0)
df1 = pd.read_csv(path1)
# pm 
df2 = pd.read_csv(path2)
df3 = pd.read_csv(path3)

threads= df0["thread"].to_numpy()

no_helper_load_dram_lat=df0[" load phase Avg latency"].to_numpy()
helper_load_dram_lat=df1[" load phase Avg latency"].to_numpy()
no_helper_load_pm_lat=df2[" load phase Avg latency"].to_numpy()
helper_load_pm_lat=df3[" load phase Avg latency"].to_numpy()

no_helper_load_dram_bw=df0[" load phase Throughput"].to_numpy()
helper_load_dram_bw=df1[" load phase Throughput"].to_numpy()
no_helper_load_pm_bw=df2[" load phase Throughput"].to_numpy()
helper_load_pm_bw=df3[" load phase Throughput"].to_numpy()

x = np.arange(len(threads))  # the label locations
width = 0.35  # the width of the bars

fig, (ax0, ax1, ax2, ax3) = plt.subplots(1, 4, figsize = (24,5))



plt.setp(ax0.get_xticklabels(), fontsize=16)
plt.setp(ax0.get_yticklabels(), fontsize=16)
plt.setp(ax1.get_xticklabels(), fontsize=16)
plt.setp(ax1.get_yticklabels(), fontsize=16)
plt.setp(ax2.get_xticklabels(), fontsize=16)
plt.setp(ax2.get_yticklabels(), fontsize=16)
plt.setp(ax3.get_xticklabels(), fontsize=16)
plt.setp(ax3.get_yticklabels(), fontsize=16)

ax2.bar(x - width/2, no_helper_load_dram_lat, width, label='CCEH', fill = False, hatch="//")
ax2.bar(x + width/2, helper_load_dram_lat, width, label='CCEH with prefetching', color = "black")


ax0.bar(x - width/2, no_helper_load_pm_lat, width, label='CCEH', fill = False, hatch="//")
ax0.bar(x + width/2, helper_load_pm_lat, width, label='CCEH with prefetching', color = "black")

ax3.bar(x - width/2, no_helper_load_dram_bw, width, label='CCEH', fill = False, hatch="//")
ax3.bar(x + width/2, helper_load_dram_bw, width, label='CCEH with prefetching', color = "black")

ax1.bar(x - width/2, no_helper_load_pm_bw, width, label='CCEH', fill = False, hatch="//")
ax1.bar(x + width/2, helper_load_pm_bw, width, label='CCEH with prefetching', color = "black")
# Add some text for labels, title and custom x-axis tick labels, etc.
ax0.set_ylabel('Latency(CPU cycle)',fontsize = 16)
ax1.set_ylabel('Throughput(Mops/s)',fontsize = 16)
ax2.set_ylabel('Latency(CPU cycle)',fontsize = 16)
ax3.set_ylabel('Throughput(Mops/s)',fontsize = 16)

ax0.set_xticks(x)
ax0.set_xticklabels(threads.astype(int))
ax2.set_xticks(x)
ax2.set_xticklabels(threads.astype(int))
ax1.set_xticks(x)
ax1.set_xticklabels(threads.astype(int))
ax3.set_xticks(x)
ax3.set_xticklabels(threads.astype(int))

ax0.set_title('(a) Latency on PM', fontsize = 16, y=-0.4)
ax2.set_title('(c) Latency on DRAM', fontsize = 16, y=-0.4)
ax1.set_title('(b) Throughput on PM', fontsize = 16, y=-0.4)
ax3.set_title('(d) Throughput on DRAM', fontsize = 16, y=-0.4)
ax0.set_xlabel("Number of workers",fontsize = 16)
ax2.set_xlabel("Number of workers",fontsize = 16)
ax1.set_xlabel("Number of workers",fontsize = 16)
ax3.set_xlabel("Number of workers",fontsize = 16)
ax0.grid(axis= 'y',linestyle='--')
ax2.grid(axis= 'y',linestyle='--')
ax1.grid(axis= 'y',linestyle='--')
ax3.grid(axis= 'y',linestyle='--')
# ax.bar_label(latency_no_preread, padding=3)
# ax.bar_label(latency_preread, padding=3)

# ax0.legend(fontsize = 16, loc='upper center', ncol = 2, bbox_to_anchor=(0.5, 1.1))

handles, labels = ax0.get_legend_handles_labels()
fig.legend(handles, labels, fontsize = 16, loc='upper center', ncol = 2, bbox_to_anchor=(0.5, 1.11))

fig.tight_layout()

plt.show()
foo_fig = plt.gcf() # 'get current figure'
foo_fig.savefig(os.path.join(this_file_dir, "..", "output", "case_study", "cceh.png"), bbox_inches='tight', format='png', dpi=1000,pad_inches=0.0)