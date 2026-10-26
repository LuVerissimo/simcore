import matplotlib.pyplot as plt
import numpy as np

block_sizes = [32, 64, 128, 256, 512, 1024]
achieved_occ = [48, 96, 94, 88, 82, 80]
bandwidth = [193, 377, 713, 1433, 1469, 1469]

fig, ax1 = plt.subplots(figsize=(10, 6))

ax1.set_xlabel('Block Size (threads)')
ax1.set_ylabel('Bandwidth (GB/s)', color='tab:blue')
ax1.plot(block_sizes, bandwidth, 'o-', color='tab:blue', linewidth=2, markersize=8, label='Bandwidth')
ax1.tick_params(axis='y', labelcolor='tab:blue')
ax1.set_xscale('log', base=2)
ax1.set_xticks(block_sizes)
ax1.set_xticklabels(block_sizes)

ax2 = ax1.twinx()
ax2.set_ylabel('Achieved Occupancy (%)', color='tab:red')
ax2.plot(block_sizes, achieved_occ, 's--', color='tab:red', linewidth=2, markersize=8, label='Achieved Occupancy')
ax2.tick_params(axis='y', labelcolor='tab:red')
ax2.set_ylim(0, 100)

fig.suptitle('Warp Shuffle Reduction: Occupancy vs Bandwidth (A100, N=100M)', fontsize=13)
fig.legend(loc='upper left', bbox_to_anchor=(0.12, 0.88))
fig.tight_layout()
plt.savefig('docs/occupancy_bandwidth.png', dpi=150)
print('Saved to docs/occupancy_bandwidth.png')
