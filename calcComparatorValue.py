#!/usr/bin/python

# Calculate 16MHz ATMega 328P Timer comparator value for desired sampling 
# frequency and chosen clock prescaler

import sys

if len(sys.argv)<=1:
	print "Usage:"
	print "%s  fsampling[Hz] prescaler{8, 32, 64, 128, 256, 1024}" \
	       %sys.argv[0]
	exit()

fsampling = int(sys.argv[1])
prescaler = int(sys.argv[2])

comparatorValue = (16000000.0/(prescaler*fsampling)) - 1

print "Comparator Value: %i"%comparatorValue
