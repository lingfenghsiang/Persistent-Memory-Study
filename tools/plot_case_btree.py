import matplotlib.pyplot as plt
import numpy as np
import matplotlib.ticker as ticker
import pandas as pd
import os

this_file_dir = os.path.abspath(os.path.dirname(__file__))

mks = ["o", "v", "^", "<", ">", "s", "p", "*", "D", "P"]
lineType = ["-", "--", ":", "-."]

data_path0 = os.path.join(this_file_dir, "..", "output",
                          "case_study", "fastfair_original_test.csv")
data_path1 = os.path.join(this_file_dir, "..", "output",
                          "case_study", "fastfair_rap_mod_test.csv")

# path0 = "./g1_fastfair_mod_pm_sfence.txt"
# path1 = "./g1_fastfair_do_flush_pm_sfence.txt"
# path2 = "./g2_fastfair_mod_pm_sfence.txt"
# path3 = "./g2_fastfair_do_flush_pm_sfence.txt"


df0 = pd.read_csv(data_path0)
df1 = pd.read_csv(data_path1)

threads = df0["thread"].to_numpy()

pm_mod_throughput = df1[" load phase Throughput"].to_numpy()
pm_do_flush_throughput = df0[" load phase Throughput"].to_numpy()
pm_mod_latency = df1[" load phase Avg latency"].to_numpy()
pm_do_flush_latency = df0[" load phase Avg latency"].to_numpy()

# fig, ax1 = plt.subplots(figsize = (8,4.96))
fig, ((ax0, ax1)) = plt.subplots(1, 2, figsize=(10, 3.7))


ax0.plot(threads, pm_mod_throughput, "o"+lineType[0], label="Out-of-place update",
         markerfacecolor='none', color="black", markersize=10, linewidth=2)
ax0.plot(threads, pm_do_flush_throughput, "x"+':', label="In-place update",
         markerfacecolor='none', color="black", markersize=10, linewidth=2)
ax1.plot(threads, pm_mod_latency/1000, "o"+lineType[0], label="Out-of-place update",
         markerfacecolor='none', color="black", markersize=10, linewidth=2)
ax1.plot(threads, pm_do_flush_latency/1000, "x"+':', label="In-place update",
         markerfacecolor='none', color="black", markersize=10, linewidth=2)


ax0.grid(linestyle='--', axis='y')
ax1.grid(linestyle='--', axis='y')

the_font_size = 12

# ax0.set_xlabel("# of thread", fontsize = 10)
ax0.set_ylabel("Throughput\n(Mops/s)", fontsize=the_font_size)
# ax1.set_xlabel("# of thread", fontsize = 10)
# ax1.set_ylabel("Throughput\n(Mops/s)", fontsize = 10)
ax0.set_xlabel("# of thread", fontsize=the_font_size)
ax1.set_ylabel("Latency\n(X1000 CPU cycles)", fontsize=the_font_size)
ax1.set_xlabel("# of thread", fontsize=the_font_size)
# ax3.set_ylabel("Latency\n(X1000 CPU cycles)", fontsize = 16)


ax0.tick_params(axis='y', labelsize=the_font_size)
ax0.tick_params(axis='x', labelsize=the_font_size)
ax1.tick_params(axis='y', labelsize=the_font_size)
ax1.tick_params(axis='x', labelsize=the_font_size)

space0 = -0.3
space1 = -0.5

ax0.set_title('(a) Optane\nThroughput', fontsize=the_font_size, y=space0)
ax1.set_title('(b) Optane\nLatency', fontsize=the_font_size, y=space0)


handles, labels = ax0.get_legend_handles_labels()
fig.legend(handles, labels, fontsize=the_font_size,
           loc='upper center', ncol=2, bbox_to_anchor=(0.55, 1.09))

# ytics= np.arange(0,1.5,0.2)
# ax0.set_yticks(ytics)
# ytics= np.arange(0,40,5)
# ax2.set_yticks(ytics)
# xtics= np.arange(1,10,2)
# ax2.set_xticks(xtics)
# ax1.set_xticks(xtics)

# ax0.sharey(ax0)
# ax3.sharey(ax2)
# ax0.sharex(ax2)
# ax3.sharex(ax1)

fig.tight_layout(pad=0)
# ytics= np.arange(0,40,5)
# ax1.set_yticks(ytics)

foo_fig = plt.gcf()  # 'get current figure'
foo_fig.savefig(os.path.join(this_file_dir, "..", "output", "case_study", "fastfair.png"),
                bbox_inches='tight', format='png', dpi=1000, pad_inches=0.0)
