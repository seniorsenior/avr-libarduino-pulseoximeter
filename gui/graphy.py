#!/usr/bin/python

import sys
from PyQt4 import QtGui, QtCore
from math import sin,pi
import struct
import serial

#sample_period = 0.032768 # sample every 2 overflows
sample_period = 0.06553599 # sample every 4 overflows
BUFFERSIZE = 100
WINDOWSIZE = 600

def find_peaks(array, x):
    """looks through array and returns all of the offsets which correspond
    to a positive peak.  Uses x to determine how far on either direction 
    to look.
    """
    results = []
    for i in range(len(array)):
        is_peak = 1
        for j in range(x):
            # make sure we are in bounds
            if ((i-j) < 0):
                continue
            if ((i+j) > (len(array)-1)):
                continue

            if (array[i] < array[i-j]):
                is_peak = 0
            if (array[i] < array[i+j]):
                is_peak = 0

            for r in results:
                if (abs(i-r) < x):
                    is_peak = 0
        if is_peak:
            results.append(i)
    return results

class Graph(QtGui.QLabel):
    def __init__(self, parent):
        QtGui.QLabel.__init__(self, parent)
        self.setMinimumSize(WINDOWSIZE, 512)
        self.setMaximumSize(WINDOWSIZE, 512)
        self.parent = parent
        self.num = [75, 150, 225, 300, 375, 450, 525, 600, 675]
        self.heart = QtGui.QImage("heart.png")

    def paintEvent(self, event):
        paint = QtGui.QPainter()
        paint.begin(self)

        font = QtGui.QFont('Serif', 7, QtGui.QFont.Light)
        paint.setFont(font)

        size = self.size()
        w = size.width()
        h = size.height()
        cw = self.parent.cw
        heartrate = self.parent.heartrate
        maxLabel = self.parent.maxLabel
        minLabel = self.parent.minLabel
        maxIRLabel = self.parent.maxIRLabel
        minIRLabel = self.parent.minIRLabel
        data_red = self.parent.data_red
        data_ir = self.parent.data_ir
        step = int(round(w / 10.0))

        data_hr = []

        till = int(((w / 750.0) * cw))
        full = int(((w / 750.0) * 700))

        #for i in range(600):
        #    data[i] = ((sin(i*cw)*128) + 128)

        #if cw >= 700:
        #    paint.setPen(QtGui.QColor(255, 255, 255))
        #    paint.setBrush(QtGui.QColor(255, 255, 184))
        #    paint.drawRect(0, 0, full, h)
        #    paint.setPen(QtGui.QColor(255, 175, 175))
        #    paint.setBrush(QtGui.QColor(255, 175, 175))
        #    paint.drawRect(full, 0, till-full, h)
        #else:
        #    paint.setPen(QtGui.QColor(255, 255, 255))
        #    paint.setBrush(QtGui.QColor(255, 255, 184))
        #    paint.drawRect(0, 0, till, h)

        # background
        paint.setPen(QtGui.QColor(255, 255, 255))
        paint.setBrush(QtGui.QColor(255, 255, 255))
        paint.drawRect(0, 0, w, h)

        # TODO: do the auto scaling for ir also
        # get our min and max for auto scalling
        max = 0
        min = 99999
        for i in range(BUFFERSIZE):
            if ((data_red[i] < 6000) and (data_red[i] > max)):
                max = data_red[i]
            if ((data_red[i] > 0) and (data_red[i] < min)):
                min = data_red[i]
        #min = 0 # for calibration

        max_ir = 0
        min_ir = 99999
        for i in range(BUFFERSIZE):
            if ((data_ir[i] < 6000) and (data_ir[i] > max_ir)):
                max_ir = data_ir[i]
            if ((data_ir[i] > 0) and (data_ir[i] < min_ir)):
                min_ir = data_ir[i]
        #min_ir = 0 # for calibration

        # draw our data
        # RED LED
        paint.setPen(QtGui.QColor(255, 0, 0))
        paint.setBrush(QtGui.QColor(255, 255, 255))
        #print str(min) + " - " + str(max)
        maxLabel.setText("Max Red \n" + str(max))
        minLabel.setText("Min Red \n" + str(min))
        maxIRLabel.setText("Max IR \n" + str(max_ir))
        minIRLabel.setText("Min IR \n" + str(min_ir))
        for i in range(BUFFERSIZE-1):
            value = (data_red[i] - (min))
            next_value = (data_red[i+1] - min)
            paint.drawLine(i*(WINDOWSIZE/BUFFERSIZE), 512-value, (i+1)*(WINDOWSIZE/BUFFERSIZE), 512-next_value)

        # IR LED
        paint.setPen(QtGui.QColor(0, 0, 255))
        paint.setBrush(QtGui.QColor(255, 255, 255))
        for i in range(BUFFERSIZE-1):
            value_ir = (data_ir[i] - (min_ir))
            next_value_ir = (data_ir[i+1] - min_ir)
            paint.drawLine(i*(WINDOWSIZE/BUFFERSIZE), 512-value_ir, (i+1)*(WINDOWSIZE/BUFFERSIZE), 512-next_value_ir)

        # find heartbeats
        paint.setPen(QtGui.QColor(0, 255, 0))
        paint.setBrush(QtGui.QColor(255, 255, 255))
        peaks = find_peaks(data_red, cw)
        last_peak = 0;
        heartrates = []
        for i in peaks:
            if (data_red[i] > 0):
                #paint.drawLine(i, 0, i, 511)
                paint.drawImage((i*(WINDOWSIZE/BUFFERSIZE))-9, 503 - ((data_red[i] - min)), self.heart)
            if len(peaks) < 4: # skip if we don't have enough data
                continue
            if (i < cw): # skip the first
                continue
            if ((i + cw) > BUFFERSIZE): # skip the last
                continue
            hr = (1 / ((i - last_peak) * sample_period)) * 60.0
            if ((hr > 30) and (hr < 220)): # only reasonable hr
                heartrates.append(hr)
                data_hr.append([i,hr])
                # print hr
            last_peak = i

        # sort, chop out some median values, then take the average for our HR
        if (len(heartrates) > 4):
            heartrates.sort()
            avg_hr = sum(heartrates[len(heartrates)/2-2:len(heartrates)/2+2]) / 4.0
            # print "average hr = " + str(avg_hr)
            heartrate.setText("Heartrate\n" + str(int(round(avg_hr))))

            pen = QtGui.QPen(QtGui.QColor(20, 20, 20), 1, QtCore.Qt.SolidLine)
            paint.setPen(pen)
            paint.setBrush(QtCore.Qt.NoBrush)
            paint.drawRect(0, 0, w-1, h-1)

        j = 0

        #for i in range(step, 10*step, step):
        #    paint.drawLine(i, 0, i, 5)
        #    metrics = paint.fontMetrics()
        #    fw = metrics.width(str(self.num[j]))
        #    paint.drawText(i-fw/2, h/2, str(self.num[j]))
        #    j = j + 1

        paint.end()

class MainWindow(QtGui.QWidget):
    def __init__(self, parent=None):
        QtGui.QWidget.__init__(self, parent)

        self.cw = 7

        # set up a buffer to sore our data
        self.data_red = []
        self.data_ir = []
        for i in range(BUFFERSIZE):
            #self.data.append((sin(i*self.cw)*128) + 128)
            self.data_red.append(0)
            self.data_ir.append(0)

        self.thread = Worker(self)

        self.slider = QtGui.QSlider(QtCore.Qt.Horizontal, self)
        self.slider.setFocusPolicy(QtCore.Qt.NoFocus)
        self.slider.setRange(1, 20)
        self.slider.setValue(self.cw)
        self.slider.setGeometry(30, 40, 150, 30)

        self.heartrate = QtGui.QLabel("Heartrate\n?", self)
        self.maxLabel = QtGui.QLabel("Max Red \n?", self)
        self.minLabel = QtGui.QLabel("Min Red \n?", self)
        self.maxIRLabel = QtGui.QLabel("Max IR \n?", self)
        self.minIRLabel = QtGui.QLabel("Min IR \n?", self)
        self.heartrate.setAlignment(QtCore.Qt.AlignHCenter)
        self.maxLabel.setAlignment(QtCore.Qt.AlignHCenter)
        self.minLabel.setAlignment(QtCore.Qt.AlignHCenter)
        self.maxIRLabel.setAlignment(QtCore.Qt.AlignHCenter)
        self.minIRLabel.setAlignment(QtCore.Qt.AlignHCenter)

        self.wid = Graph(self)

        self.button = QtGui.QPushButton('Start',self)
        self.button.setGeometry(10, 10, 60, 35)

        hbox_top = QtGui.QHBoxLayout()
        hbox_top.addWidget(self.button)
        #hbox_top.addWidget(self.slider)
        hbox_top.addWidget(self.minLabel)
        hbox_top.addWidget(self.maxLabel)
        hbox_top.addWidget(self.minIRLabel)
        hbox_top.addWidget(self.maxIRLabel)
        hbox_top.addWidget(self.heartrate)

        hbox = QtGui.QHBoxLayout()
        hbox.addWidget(self.wid)
        vbox = QtGui.QVBoxLayout()
        vbox.addStretch(1)
        vbox.addLayout(hbox_top)
        vbox.addWidget(self.slider)
        vbox.addLayout(hbox)
        self.setLayout(vbox)

        self.connect(self.slider, QtCore.SIGNAL('valueChanged(int)'), self.changeValue)
        self.connect(self.thread, QtCore.SIGNAL('newData()'), self.newData)
        self.connect(self.button, QtCore.SIGNAL('clicked()'), self.thread, 
            QtCore.SLOT('start()'))

        self.setGeometry(0, 0, 600, 600)
        self.setWindowTitle('Pulse Oximeter Data')

    def changeValue(self, event):
        self.cw = self.slider.value()
        print self.cw
        self.wid.repaint()

    def newData(self):
        self.button.setEnabled(0)
        self.button.setText('Running')
        self.wid.repaint()

class Worker(QtCore.QThread):

    def __init__(self, parent = None):
        QtCore.QThread.__init__(self, parent)
        self.parent = parent

    def run(self):
        data_red = self.parent.data_red
        data_ir = self.parent.data_ir
        dataptr_red = 0
        dataptr_ir = 0
        print "thread started"
        #with open('/tmp/testfifo', 'rb') as f:
        #    while True:
        #        byte = f.read(1)
        #    if not byte:
        #        break
        #    data[dataptr] = ord(byte)
        #    dataptr = (dataptr + 1) % len(data)
        #        print byte
        f = serial.Serial('/dev/ttyUSB0', 57600, timeout=1)
        for line in f:
            line = line.strip()
            [type,number] = line.split('=')
            if (type == "r"):
                value = int(number) 
                data_red[dataptr_red] = value
                dataptr_red = (dataptr_red + 1) % len(data_red)
            if (type == "w"):
                value = int(number)
                data_ir[dataptr_ir] = value
                dataptr_ir= (dataptr_ir + 1) % len(data_ir)
            if not (dataptr_red % 4):
                self.emit(QtCore.SIGNAL('newData()'))
            #print value
            f.close()
            print "thread done"

app = QtGui.QApplication(sys.argv)
mw = MainWindow()
mw.show()
app.exec_()

