#!/usr/bin/env python
from __future__ import print_function
from __future__ import absolute_import

import os
import csv
import sys
import glob
import operator
import numpy as np
import itertools, functools
import matplotlib.pyplot as plt

from sklearn import preprocessing
from scipy.optimize import curve_fit

from waveread import read_wfm_file_normalized

datapath = '../analytics/train_data.csv'
trainpath = '../analytics/train_ans.csv'
processed_dir = '../analytics/processed_files/'
test_path = '../control_waves'
bus_classes = ["I2C", "SPI", "ETHERNET", "MIPI", "UART", "RS232", "FLEXRAY","MIL1553",
                       "CAN", 'LIN', "USB"
                       ]
def get_color():
    """returns a random 3 tuple for random RGB coloring"""
    return (np.random.rand(), np.random.rand(), np.random.rand())

def wave_bitrate(edges):
    try:
        bitrate, period = unit_interval(edges)
    except TypeError:
        return (None, None)
    (1/bitrate)+.0001
    #print ("bitrate: {}\t\n".format(bitrate))
    return bitrate, period

def to_edges(array, maximum=None, minimum=None, level=None, hysteresis=None):

    if level is None or hysteresis is None:
        if level is None:
            level = (maximum - minimum) / 2 + minimum
        if hysteresis is None:
            hysteresis = (maximum - minimum) * 0.1
    high = level + hysteresis
    low = level - hysteresis
    initial = array[0] >= level
    out = edges_with_hysteresis(array, high, low, level, initial)

    return out

def edges_with_hysteresis(x, high_level, low_level, mid_level, initial=False):

    # find the indexes that are above the high threshold
    hi = x >= high_level
    # find the indexes that are below the low threshold
    lo = x <= low_level
    # combine them to find the locations where an edge would be valid
    lo_or_hi = lo | hi

    # find the valid indexes to locate
    ind = np.nonzero(lo_or_hi)[0]
    if not ind.size:  # prevent index error if ind is empty
        indexes_precise = np.empty(shape=(0, 0), dtype=float)
    else:
        non_hysteresis_indecies = np.cumsum(lo_or_hi)  # from 0 to len(x)
        edge_locations = np.where(non_hysteresis_indecies,
                         hi[ind[non_hysteresis_indecies-1]], initial)
        edge_indexes = np.where(edge_locations[:-1] != edge_locations[1:])[0]
        first_samples = x[edge_indexes]
        second_samples = x[edge_indexes + 1]
        # find the slope of every edge in the system
        slope = first_samples - second_samples
        # if the edge is rising, calculate the subsample against th_hi, 
        # otherwise compare against th_lo
        # afterward, divide by the slope to determine the edge subsammple position
        subsample = np.where(slope < 0, (high_level - first_samples),
                    low_level - first_samples) / slope
        # subsample = (th_hi - first_samples) / slope
        indexes_precise = edge_indexes - subsample
    return indexes_precise, high_level, low_level, mid_level, initial

def unit_interval(edges):

    if np.size(edges) < 2:
        return float(np.inf)
    #calculate all the edge periods
    difference = np.diff(edges)
    period = functools.reduce(operator.add, difference)/len(difference)
    assert type(period) is np.float64
    #estimate using the smallest period as the bitrate
    smallest_period = difference.min(axis=0)
    #use to find all single period samples
    single_cycle_periods = difference[np.where(difference<(1.5 * smallest_period))]
    average_single_cycle_period = np.average(single_cycle_periods)
    current_average = 0.0
    number_of_samples = 0
    #for 1, 2, 3, and 4 ui periods, estimate the bitrate
    for number_of_ui in range(1, 5):
        range_start = (number_of_ui - 0.5) * average_single_cycle_period
        range_end = (number_of_ui + 0.5) * average_single_cycle_period
        periods = difference[np.where((range_start <= difference) &
                  (range_end > difference))]
        if len(periods) == 0:
            this_average = 0
        else:
            this_average = np.average(periods/number_of_ui)
        this_number_of_samples = np.size(periods)
        # use boxcar averaging to add samples inline
        current_average = current_average * number_of_samples + this_average * \
                          this_number_of_samples
        number_of_samples += this_number_of_samples
        current_average /= number_of_samples

    return current_average, period

def level_finder(samples):

    size = 3
    hist = np.histogram(samples, bins=512)
    hist_len = len(hist[0])
    min_score = np.inf
    min_bin = None
    levels = [2, 3, 4, 5, 8]
    mu = lambda x, y: sum(x*window_scaled)/len(x)
    sigma = lambda x, y, mean: sum(y*(x-mean)**2)/len(x)
    gaussian = lambda x, a, x0, std: a*np.exp(-(x-x0)**2/(2*std**2))

    for level in levels:
        error = []
        for num in range(level):
            # add up the error of each guess
            start = int(num * hist_len / level)
            end = int((num + 1) * hist_len / level) - 1
            window = hist[0][start:end]
            # normalize window to 0-1
            min_max_scaler = preprocessing.MinMaxScaler(feature_range=(0,1))
            to_scale = list(map(float, window))
            assert type(to_scale) == list
            window_scaled = min_max_scaler.fit_transform(to_scale)
            x = np.arange(len(window_scaled))
            # compute a rolling mean and standard deviation
            mean = mu(x, window_scaled)
            std = sigma(x, window_scaled, mean)
            if std <= 0 :
                #warnings.warn("std deviation must be > 0", RuntimeWarning)
                return -1
            # fit a normal distribution with same params
            try :
                popt, pcov = curve_fit(gaussian, x, window_scaled, p0=[.5,mean,std])
            except :
                #print ("Optimize warning, could not fit params for {}".format(num))
                continue
            fit = gaussian(x,*popt)

            RMSerror = 0
            for x, y in zip(fit, window_scaled):
                RMSerror +=(x-y)**2
            RMSerror = np.sqrt(RMSerror/min(len(fit), len(window_scaled)))
            error.append(RMSerror)
        if len(error) == 0:
            warnings.warn("No error from normal distribution, possibly result \
                          of runtime warning",
                          RuntimeWarning)
            return -1
        else:
            average = sum(error)/len(error)

        #print ("score for {} levels is {}".format(level, average))
        if average < min_score:
            min_score   = average
            min_bin     = level

    #print ("levels found with a gaussian filter: {}".format(min_bin))
    return min_bin

def convolve(x, window_len, window='flat'):
    """smooths 1D arrays with a windowing average
    types of windows availible:
    flat, hanning, hamming, bartlett, blackman
    returns smoothed array"""

    x = np.array(x)
    if x.ndim != 1:
        raise ValueError("smooth only accepts 1 dimension arrays.")
    if len(x) < window_len:
        return x
    if window_len < 3:
        return x
    if not window in ['flat', 'hanning', 'hamming', 'bartlett', 'blackman']:
        raise ValueError("Window is not an accepted type")

    s=np.r_[x[window_len-1:0:-1],x,x[-1:-window_len:-1]]

    if window == 'flat': #moving average
        w=np.ones(window_len,'d')
    else:
        w=eval('np.'+window+'(window_len)')
    y=np.convolve(w/w.sum(),s,mode='valid')
    res = y[(window_len/2-1):-(window_len/2)]
    return res

def histogram_levels(y):
    count, bins, ignored = plt.hist(y, 100, normed=True)
    return (count, bins)

def binned_average(y, max_value, min_value, levels, window=3):

    # divide up the amplitude into horizontal bins #
    N_bins = 16
    bin_top = max_value
    bin_bottom = min_value
    amplitude = max_value-min_value
    step_size = amplitude / N_bins
    bins = np.linspace(bin_bottom, bin_top, N_bins, endpoint=True)

    # split data into what falls into these bins #
    # logical & marries the two conditions in numpy #
    y_split, count = [], 0
    for i in range(N_bins-1):
        if i is 0:
            a = ((y >= bins[0]) & (y <= bins[i+1]))
        else:
            a = ((y > bins[i]) & (y <= bins[i+1]))

        y_split.append(y[a])
        count += len(y_split[i])

    # sort the subarrays by their length to find the longest intervals inside bins #
    y_split.sort(key=len, reverse=True)
    y_combined = check_transitions(y_split[:7], amplitude, levels)
    y_smooth = [convolve(y_combined[i], window, 'hanning')
                        for i in range(len(y_combined))]
    for i in y_smooth:
        try :
            x, y = min(i), max(i)
        except :
            y_smooth = np.delete(y_smooth, (i), axis=0)
    return y_smooth

def check_transitions(y, amplitude, levels):

    p = .1 * amplitude
    y = list(map(list, y))
    def compare(y, p, x, marked):
        avgs = [np.average(q) for q in y]
        for i, lst in enumerate(y):
            matches = []
            if lst in marked:
                continue
            #iterate through averages to compare with y
            for idx, avg in enumerate(avgs):
                #test to see if level is close to another
                cmp = abs(np.average(lst) - avg)
                #if level is close and the level isn't already marked off
                if (0<cmp<p) and (y[idx] not in marked):
                    matches.append(y[idx])
                    marked.append(y[idx])

            #averages are done looping, mark element, and add to matches 
            marked.append(lst)
            matches.append(lst)
            x.append(list(itertools.chain(*matches)))
            mark = [np.average(f) for f in marked]
        filtered_levels = sorted(x, key=len, reverse=True)
        filtered_levels = x[::-1]
        return filtered_levels

    x = compare(y, p, [], [])
    #print ("\n###\t{} new levels \t###\n".format(len(x)))
    return x if len(x) != 0 else y

def normal(y):
    mean = np.mean(y)
    std = np.std(y)
    return (mean, std)

def write_out(train_data, train_answers):
    global testdir, seccessful_dir
    with open(datapath, 'wb') as csvfile:
        writer = csv.writer(csvfile, delimiter=' ')
        writer.writerow(['bitrate','amplitude','max','min', 'std','mean',
                         'sample_rate', 'levels'])
        map(writer.writerow, train_data)

    with open(trainpath, 'wb') as trainfile:
        writer = csv.writer(trainfile, delimiter=' ')
        writer.writerow(["Busses"])
        map(writer.writerow, train_answers)

    # move completed file to the successful_dir
    print ("{} files processed, data contained in /analytics/".format(len(train_data)))

def process_waveform(pathname):

    matched_path = match_file(pathname)
    print ("retreiving data for {}:".format(matched_path[0]))
    path = matched_path[0]
    try:
        waveform = read_wfm_file_normalized(pathname) # extract waveform data
    except:
        warnings.warn("Incompatible wfm file", RuntimeWarning)
        return (-1, -1)
    y = np.array(waveform.vertical) # raw adc values
    x = np.arange(len(y))
    minimum = np.amin(y)
    maximum = np.amax(y)
    fit_level = level_finder(y)
    if fit_level == -1:
        warnings.warn("Could not determine signal levels, will attempt to continue",
                      RuntimeWarning)

    bin_data = binned_average(y, maximum, minimum, fit_level)
    bin_level = len(bin_data)
    edges = to_edges(y, maximum, minimum) # get a list of edges
    bitrate = wave_bitrate(edges[0])
    amplitude = maximum - minimum
    sample_rate = waveform.sample_rate
    mean = sum(x*y)/len(x)
    sigma = sum(y*(x-mean)**2)/len(x)
    bus = [matched_path[1]]
    wavedata = [bitrate,amplitude,maximum,minimum,sigma,mean,
                sample_rate,bin_level]
    print ("moving file to /analytics/processed_files")
    os.rename(path, processed_dir+path[17:])
    print (wavedata)
    return (wavedata, bus)

def scope_process(waveform):

    y = np.array(waveform.vertical) # raw adc values
    x = np.arange(len(y))
    minimum = np.amin(y)
    maximum = np.amax(y)
    fit_level = level_finder(y)
    if fit_level == -1:
        warnings.warn("Could not determine signal levels, will attempt to continue",
                      RuntimeWarning)
    bin_data = binned_average(y, maximum, minimum, fit_level)
    bin_level = len(bin_data)
    edges = to_edges(y, maximum, minimum) # get a list of edges
    bitrate, period = wave_bitrate(edges[0])
    if bitrate is None:
        return None
    amplitude = maximum - minimum
    sample_rate = waveform.sample_rate
    if bitrate > sample_rate:
        return None
    mean = sum(x*y)/len(x)
    sigma = sum(y*(x-mean)**2)/len(x)
    wavedata = [bitrate,amplitude,maximum,minimum,sigma,mean,
                sample_rate, bin_level, period]
    return wavedata

def match_file(path):
    matched_path = []
    pairs = zip(range(len(bus_classes)), bus_classes)
    numeric_bus = dict((v, k) for k, v in pairs)
    wfm = path
    f = wfm.upper()
    for bus in numeric_bus.items():
        if bus[0] in f:

            print ("FILE MATCHED", '\t', wfm, ':', bus[0], '-', bus[1])
            matched_path = (wfm, bus[1])
            break

    return matched_path

