import glob
import sys
import numpy as np
import matplotlib.pyplot as plt
import wave
import os
import struct

def main():
    energy_matches = []
    frequency_matches = []

    for f in glob.glob("./*.wav"):

        wav = wave.open(f, 'rb')
        channels = wav.getnchannels()
        fs = wav.getframerate()
        wavelen = wav.getnframes()
        y_struct = wav.readframes(wavelen-1)
        print len(y_struct)
        y = np.array(struct.unpack("%dh" % (len(y_struct)/2), y_struct))
        x = np.linspace(0, 1./fs * len(y), len(y))
        y_fft = np.fft.fft(y)
        print "{} sample points".format(len(y))
        x_fft = np.linspace(0, fs, len(y))
        plt.plot(x_fft, np.abs(y_fft))
        plt.show()


if __name__ == '__main__':
    main()


