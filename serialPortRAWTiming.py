#!/usr/bin/python

import serial
import struct

nreps=50000

ser = serial.Serial('/dev/tty.usbmodem1421', 115200*2)
counter = 1 
#while True:
for i in range(nreps):
	data = struct.unpack('BBBBBB',ser.read(size=6))

	print counter, data
	counter +=1
while 1:
	data = ser.readline().rstrip()
	print data

                             
