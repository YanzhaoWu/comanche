#!/usr/bin/python3
import glob
import re
import numpy as np
import sys
import matplotlib.pyplot as plt
from matplotlib import mlab

def read_file(file):
    lines = [line.rstrip('\n') for line in open(file)]
    data = [(float(line)/2400.0) for line in lines]
    return data
    

# def plot((xdat,ydat),label_param, format, color):
#     N = len(ydat)
#     width = 1/1.5
# #    plt.yscale('log')
# #    plt.xscale('log')
# #    plt.gca().invert_xaxis()
# #    plt.ylim([0,ydat[16]+5])
#     plt.xlim([0,105])
#     plt.xlabel('Percentile')
#     plt.ylabel('Completion Latency (usec)')
#     plt.title('Intel PC3700 SSD - fio + ext2 + kernel IO Latencies')
#     plt.plot(xdat,ydat,format,label=label_param)

data = read_file('./latencies.log')


mu = 200
sigma = 25
n_bins = 200
#x = mu + sigma * np.asarray(data)
x = np.asarray(data)

print(np.mean(x))

n, bins, patches = plt.hist(x, n_bins, normed=1,
                            histtype='step', cumulative=True)

# Add a line showing the expected distribution.
#y = mlab.normpdf(bins, mu, sigma).cumsum()
#y /= y[-1]
#plt.plot(bins, y, 'k--', linewidth=1.5)

plt.grid(True)
plt.ylim(0, 1.05)
plt.xlim(5,30)
plt.xlabel('Round trip latency (usec)')
plt.ylabel('Cumulative Frequency')
#plt.title('cumulative step')

plt.show()

