#!/usr/bin/env python 
from __future__ import absolute_import
from __future__ import print_function

import sys
import numpy as np
import pandas as pd
#import dsclient
import itertools, functools

from sklearn.svm import LinearSVC
from sklearn.ensemble import RandomForestClassifier as RF
from sklearn.feature_selection import SelectKBest, f_classif
from sklearn.pipeline import Pipeline

from tkinter import Tk, Text, W, N, E, S, Menu, Checkbutton, Listbox
from tkinter import TOP, BOTTOM, RIGHT, LEFT, RAISED, FLAT, END, VERTICAL, BOTH
from tkinter.filedialog import askopenfilename, asksaveasfilename
from tkinter.ttk import Button, Frame, Style, Label, Panedwindow, Labelframe
from tkinter import messagebox as box
from tkinter import simpledialog as tkd

from wavetools import scope_process
from classifier import print_feature_importance, user_prediction, build_clf
from classifier import get_wave_data, get_test_data, pull_features
from preprocessing import set_data, make_prediction, load_model, get_bus_indices

testpath = '../data/train_ans.csv'

bus_classes = ["I2C", "SPI", "ETHERNET", "MIPI", "UART", "RS232", "FLEXRAY",
               "MIL1553", "CAN", "LIN", "USB"
               ]
class Window(Frame):

    def __init__(self, parent):

        Frame.__init__(self, parent)

        self.parent = parent
        self.initUI()

    def initUI(self):

        self.parent.title("PyWave")
        self.style = Style()
        self.style.theme_use("default")
        self.user_waves, self.user_labels = [], []
        self.save_wave = True
        self.RF = None

        self.columnconfigure(1, weight=1)
        self.columnconfigure(3, pad=7)
        self.rowconfigure(3, weight=1)
        self.rowconfigure(5, pad=7)

        self.RFparams = { 'n_estimators' : 100,
                          'random_state' : 1,
                          'n_jobs'       : -1,
                          'max_depth'    : 7,
                          'oob_score'    : True,
                          'warm_start'   : True
                          }

        #### Geometry ####

        abtn = Button(self.parent, text='Capture', command=self.capture_and_store)
        abtn.pack(side=BOTTOM, expand=1, fill='x')

        tbtn = Button(self.parent, text='Train', command=self.train_model)
        tbtn.pack(side=BOTTOM, expand=1, fill='x')

        cbtn = Button(self.parent, text='Close', command=self.quit)
        cbtn.pack(side=BOTTOM, expand=1, fill='x')

        hbtn = Button(self.parent, text='Predict on Capture', command=self.capture_and_plot)
        hbtn.pack(side=BOTTOM, expand=1, fill='x')

        p = Panedwindow(self.parent, orient=VERTICAL)
        p.pack(side=LEFT)
        f1 = Labelframe(p, text='Models', width=200, height=1000)
        p.add(f1)

        self.mdlist = Listbox(self.parent, height = 900, width=400)
        self.mdlist.pack(side=LEFT)

        #### Creating top menu bars and callbacks ####
        menubar = Menu(self.parent)
        self.parent.config(menu=menubar)
        modelMenu = Menu(menubar, tearoff=0)
        menubar.add_cascade(label="Model", menu=modelMenu)
        modelMenu.add_command(label='Import Dataset', command=self.import_dataset)
        modelMenu.add_command(label='Import Model', command=self.import_model)
        modelMenu.add_command(label='Train Random Forest', command=self.rf)
        modelMenu.add_command(label='Train Gradient Boosted Classifier',
                              command=self.gbc)
        modelMenu.add_command(label='Train K-Nearest Neighbors',
                              command=self.knn)
        modelMenu.add_command(label='Save', command=self.saveas)
        modelMenu.add_command(label='Save as', command=self.saveas)

        waveMenu = Menu(menubar, tearoff=0)
        menubar.add_cascade(label="Waveform", menu=waveMenu)
        waveMenu.add_command(label="Import Waveform", command=self.import_waveform)
        waveMenu.add_command(label="Features", command=self.get_features)

        ###############################################
    def onClick(self):

        if self.var.get() == 1:
            self.master.title("")
        else:
            self.master.title("_")

    def quit(self):
        sys.exit(0)

    def saveas(self):

        f = asksaveasfile(mode='wb', defaultextension='.pkl')
        if f is None:
            return
        else:
            return

    def train_model(self):

        if len(self.user_waves) < 3:
            box.showinfo('PyWave', 'Capture At Least 3 Waveforms To Train Model')
            self.save_wave = True
        else:
            self.add_to_model()
            print(len(self.user_waves[0]), self.user_waves)
            print(len(self.user_labels), self.user_labels)
            self.user_waves = []
            self.user_labels = []

    def add_to_model(self):

        if not self.RF:
            self.import_model()
        assert len(self.user_waves) >= 3, "not enough wavedata {}".format(self.user_waves)
        self.user_waves = np.array(self.user_waves)
        self.user_waves = [arr[self.feature_mask] for arr in self.user_waves]
        self.user_labels = get_bus_indices(self.user_labels)
        self.RF_new = RF(n_estimators=100).fit(self.user_waves, self.user_labels)
        self.combine_rfs()
        self.mdlist.insert(END, "added data to model")
        print("RF_combined", self.RF, "\nestimators:", len(self.RF.steps[1][1].estimators_))

    def combine_rfs(self):

        self.RF.steps[1][1].estimators_ += self.RF_new.estimators_
        self.RF.steps[1][1].n_estimators = len(self.RF.steps[1][1].estimators_)

    def import_dataset(self):
        ftypes = [ ('CSV files', '*.csv'),
                   ('All files', '*')]
        path = askopenfilename(filetypes=ftypes)
        self.wavedata = get_wave_data(path)
        self.testdata = get_test_data(testpath)
        self.X = self.wavedata.as_matrix().astype(np.float)
        self.y = np.array(self.testdata).flatten()

    def error_code(self):
        return "0x"+str(np.random.randint(1, 100000))

    def error_box(self, error):
        box.showerror('Error', 'Error Code : ' + self.error_code() +
                      '\n\n' + error)

    def rf(self):
        try:
            self.rf = build_rf(self.X, self.y, self.wavedata)
            self.mdlist.insert(END, "Random Forest")
        except AttributeError:
            self.error_box("Import dataset to train model")

    def gbc(self, X, y, wavedata):
        try:
            self.gbc = build_gbc(self.X, self.y, self.wavedata)
            self.mdlist.insert(END, "Boosted Classifier")
        except AttributeError:
            self.error_box("Import dataset to train model")

    def knn(self):
        try:
            self.kmc = build_clf(self.X, self.y, self.wavedata)
            self.mdlist.insert(END, "K Nearest Neighbors")
        except AttributeError:
           self.error_box("Import dataset to train model")

    def import_model(self):

        ftypes = [ ('Pickled files', '*.pkl'),
                   ('All files', '*') ]
        path = askopenfilename(filetypes=ftypes)
        print ( path )
        self.RF = load_model(path)

    def capture_and_store(self):
        pass
        """dsclient.register()

        try:
            while(dsclient.wait_for_sequence()):

                rf_guess = None
                rf_greatest_prob = -np.inf
                channel1 = dsclient.Waveform("CH1")
                channel1.vertical = ((channel1.vertical - channel1.offset)*
                                    channel1.increment) + channel1.position
                self.feature_mask = [1, 2, 3, 5, 6]

                bus = tkd.askstring('PyWave', 'Bus Name')
                if bus is None:
                    break
                bus = bus.upper()
                wavedata = set_data(channel1)
                if wavedata is -1:
                    box.infobox('PyWave', 'Zoom Out To Capture Sample')
                self.user_waves.append(wavedata)
                self.user_labels.append(bus)
                self.mdlist.insert(END, "Captured {} sample".format(bus))
                break

        except KeyboardInterrupt :
            dsclient.finished_with_sequence()
            dsclient.unregister()
        finally :
            dsclient.finished_with_sequence()
            dsclient.unregister()"""

    def capture_and_plot(self):
        pass
        """dsclient.register()

        try:
            while(dsclient.wait_for_sequence()):

                rf_guess = None
                rf_greatest_prob = -np.inf
                channel1 = dsclient.Waveform("CH1")
                channel1.vertical = ((channel1.vertical - channel1.offset)
                                    *channel1.increment) + channel1.position
                self.feature_mask = [1, 2, 3, 5, 6]
                wavedata = np.array(set_data(channel1))
                wavedata = wavedata[self.feature_mask]
                print ("wavedata", len(wavedata), wavedata)
                if not self.RF:
                    self.import_model()
                if len(wavedata) < 1:
                    continue
                print("RF before pred", self.RF)
                rf_pred, rf_prob = make_prediction(self.RF, wavedata)
                print('\nRF Probability Space\t\t\tGBC Probability Space')
                for idx, rf_proba in enumerate(rf_prob[0]):
                    print (idx)
                    print (bus_classes[idx], '--', rf_proba*100, '%')
                    if rf_proba > rf_greatest_prob:
                        rf_greatest_prob = rf_proba
                        rf_guess = bus_classes[idx]
                guesses = [(idx, x) for idx, x in enumerate(rf_prob[0]) if                                  abs((x*100) - (rf_greatest_prob*100)) <= 10]
                self.mdlist.insert(END, "\n\n")
                for guess in guesses:
                    self.mdlist.insert(
                        END,
                        "{} with {} % certainty".format(
                        bus_classes[guess[0]], round((guess[1] * 100), 3)))

                print ("continuing to next capture")
                break
        except KeyboardInterrupt :
            dsclient.finished_with_sequence()
            dsclient.unregister()
        finally :
            dsclient.finished_with_sequence()
            dsclient.unregister()"""

    def import_waveform(self):
        path = askopenfile()
        self.wave = scope_process(path)

    def get_features(self):
        self.features = print_feature_importance(self.clf, self.wavedata)

def main():

    root = Tk()
    root.geometry("{}x{}+{}+{}".format(700, 500, 300, 300))
    app = Window(root)
    root.lift()
    root.mainloop()

if __name__ == "__main__":

    main()

