
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.ticker as ticker
import pandas as pd
import os

this_file_dir = os.path.abspath(os.path.dirname(__file__))

mks = ["o", "v", "^", "<", "*", "x", "p", "s", "D", "P"]
lineType = ["-", "--", ":", "-."]

data_path = os.path.join(this_file_dir, "..", "output",
                         "micro_bench", "microbench_lat.csv")
df = pd.read_csv(data_path)
xAxis = df["wss"].to_numpy()

fenced_seq_clwb = df[" clwb sfence seq Ptr cycle"].to_numpy()
fenced_rand_clwb = df[" clwb sfence rand Ptr cycle"].to_numpy()
fenced_seq_nt = df[" nt store sfence seq Ptr cycle"].to_numpy()
fenced_rand_nt = df[" nt store sfence rand Ptr cycle"].to_numpy()

unfenced_seq_clwb = df[" clwb seq Ptr cycle"].to_numpy()
unfenced_rand_clwb = df[" clwb rand Ptr cycle"].to_numpy()
unfenced_seq_nt = df[" nt store seq Ptr cycle"].to_numpy()
unfenced_rand_nt = df[" nt store rand Ptr cycle"].to_numpy()


seq_rd = df[" rd seq Ptr cycle"].to_numpy()
rand_rd = df[" rd rand Ptr cycle"].to_numpy()
calc_offset_seq_nt = df[" nt store seq Calc cycle"].to_numpy()
calc_offset_rand_nt = df[" nt store rand Calc cycle"].to_numpy()
fig, (ax0, ax1, ax2) = plt.subplots(1, 3, figsize=(24, 4.7))

ax0.plot(xAxis, fenced_seq_clwb, mks[0]+lineType[0], markerfacecolor='none',
         label="seq_clwb", color="black", markersize=10, linewidth=2)
ax0.plot(xAxis, fenced_rand_clwb, mks[1]+lineType[0], markerfacecolor='none',
         label="rand_clwb", color="black", markersize=10, linewidth=2)
ax0.plot(xAxis, fenced_seq_nt, mks[2]+lineType[1], markerfacecolor='none',
         label="seq_nt-store", color="black", markersize=10, linewidth=2)
ax0.plot(xAxis, fenced_rand_nt, mks[3]+lineType[1], markerfacecolor='none',
         label="rand_nt-store", color="black", markersize=10, linewidth=2)

ax1.plot(xAxis, unfenced_seq_clwb,
         mks[0]+lineType[0], markerfacecolor='none', color="black", markersize=10, linewidth=2)
ax1.plot(xAxis, unfenced_rand_clwb,
         mks[1]+lineType[0], markerfacecolor='none', color="black", markersize=10, linewidth=2)
ax1.plot(xAxis, unfenced_seq_nt, mks[2]+lineType[1],
         markerfacecolor='none', color="black", markersize=10, linewidth=2)
ax1.plot(xAxis, unfenced_rand_nt, mks[3]+lineType[1],
         markerfacecolor='none', color="black", markersize=10, linewidth=2)

ax2.plot(xAxis, seq_rd, mks[4]+lineType[0], markerfacecolor='none',
         label="seq_rd", color="black", markersize=10, linewidth=2)
ax2.plot(xAxis, rand_rd, mks[5]+lineType[0], markerfacecolor='none',
         label="rand_rd", color="black", markersize=10, linewidth=2)
ax2.plot(xAxis, calc_offset_seq_nt, mks[2]+lineType[1],
         markerfacecolor='none',  color="black", markersize=10, linewidth=2)
ax2.plot(xAxis, calc_offset_rand_nt,
         mks[3]+lineType[1], markerfacecolor='none', color="black", markersize=10, linewidth=2)

ax0.set_xscale('log', base=2)
ax0.set_xticks(np.logspace(12, 32, 11, base=2))
ax0.set_yticks(np.arange(0, 1400, 200.0))
ax0.tick_params(axis='y', labelsize=16)
ax0.tick_params(axis='x', labelsize=16)
ax0.set_ylabel("Latency (CPU cycle per element)", fontsize=16)
ax0.set_xlabel("Working set size (byte)", fontsize=16)

ax1.sharey(ax0)
ax1.set_xscale('log', base=2)
ax1.set_xticks(np.logspace(12, 32, 11, base=2))
ax1.set_yticks(np.arange(0, 1400, 200.0))
ax1.tick_params(axis='y', labelsize=16)
ax1.tick_params(axis='x', labelsize=16)
ax1.set_xlabel("Working set size (byte)", fontsize=16)

ax2.sharey(ax0)
ax2.set_xscale('log', base=2)
ax2.set_xticks(np.logspace(12, 32, 11, base=2))
ax2.set_yticks(np.arange(0, 1400, 200.0))
ax2.tick_params(axis='y', labelsize=16)
ax2.tick_params(axis='x', labelsize=16)
ax2.set_xlabel("Working set size (byte)", fontsize=16)

ax0.set_title('(a) Write with strict persistency', fontsize=16, y=-0.3)
ax1.set_title('(b) Write with relaxed persistency', fontsize=16, y=-0.3)
ax2.set_title('(c) Latency breakdown of pure reads and writes',
              fontsize=16, y=-0.3)


ax0.grid(linestyle='--')
ax1.grid(linestyle='--')
ax2.grid(linestyle='--')

ax0.label_outer()
ax1.label_outer()
ax2.label_outer()


fig.legend(fontsize=16, loc='upper center', ncol=6, bbox_to_anchor=(0.5, 1.02))

foo_fig = plt.gcf()  # 'get current figure'
foo_fig.savefig(os.path.join(this_file_dir, "..", "output", "micro_bench", "lat.png"),
                bbox_inches='tight', format='png', dpi=1000, pad_inches=0.0)
