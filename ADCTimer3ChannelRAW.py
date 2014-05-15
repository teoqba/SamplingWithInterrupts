#!/usr/bin/python

import serial
import struct

nreps=100

ser = serial.Serial('/dev/tty.usbmodem1421', 115200*2)
a=[] 
#while True:
for i in range(nreps):
	xH = struct.unpack('B',ser.read(size=1))
	xL = struct.unpack('B',ser.read(size=1))
	yH = struct.unpack('B',ser.read(size=1))
	yL = struct.unpack('B',ser.read(size=1))
	zH = struct.unpack('B',ser.read(size=1))
	zL = struct.unpack('B',ser.read(size=1))

	print xH, xL, yH, yL, zH, zL
while 1:
	data = ser.readline().rstrip()
	print data

                             
