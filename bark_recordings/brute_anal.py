import glob
import sys
import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
import os
import struct

def main():
    energy_matches = []
    frequency_matches = []

    for f in glob.glob("./*.wav"):
        plt.figure()
        print "Analyzing {}\n".format(f)
        fs, data = wavfile.read(f)
        channel1 = data.T[0]
        y = np.fft.fft(channel1)
        print "{} sample points".format(len(y))
        x_fft = np.linspace(0, fs/5, (len(y)/2)-1)
        plt.plot(x_fft, (np.abs(y[:(len(y)/2)-1])))
        plt.show()


if __name__ == '__main__':
    main()


