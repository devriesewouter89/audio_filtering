
import sys
import numpy as np
import matplotlib.pyplot as plt
import itertools, functools
import audiolab
from pylab import plot, show, title, xlabel, ylabel, subplot
from scipy import fft, arange

def plotSpectrum(y,Fs):

    n = len(y) # lungime semnal
    k = arange(n)
    T = n/Fs
    frq = k/T # two sides frequency range
    frq = frq[range(n/2)] # one side frequency range
    print "creating fft"
    Y = fft(y)/n # fft computing and normalization
    Y = Y[range(n/2)]
    print "creating plot"
    plot(frq,abs(Y),'r') # plotting the spectrum
    xlabel('Freq (Hz)')
    ylabel('|Y(freq)|')
    print "returning plot"

Fs = 44100;  # sampling r
x, fs, bits=audiolab.wavread('30_yards_bark_cut_3.wav')
y = x[:,1]
lungime=len(y)
timp=len(y)/44100.
t=np.linspace(0,timp,len(y))
print "creating linspace"
subplot(2,1,1)
plot(t,y)
xlabel('Time')
ylabel('Amplitude')
subplot(2,1,2)
plotSpectrum(y,Fs)
show()

