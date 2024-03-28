
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec

matplotlib.use("TkAgg")


x = None
fig = None
gs = None
ax1 = None
ax2 = None
ax3 = None

flag_bf = None
flag_mic = None

bf_data = np.zeros([30,40])
rms_data = np.zeros(112)
mic_data = np.zeros(8000)
bf_mic_data = np.zeros(8000)

image = None
line2 = None
line3 = None

x = np.linspace(0,GLOB.TS * (GLOB.VLEN-1),GLOB.VLEN)