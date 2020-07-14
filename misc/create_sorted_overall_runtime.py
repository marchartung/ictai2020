#!/bin/python

import sys

assert len(sys.argv) > 1

f = open(sys.argv[1], "r")
prefix = sys.argv[1]
rTimes = list()
#print(prefix + ",benchmark,solve mem,solve time,drat kb,drat sec,lrat kb,lrat sec,restarts,decisions,conflicts,propagations,mark proof sec,dump lrat sec, ana sec, anamem mb")

for l in f:
    
    data = l.split(",")
    if len(data) > 15 :
        rTimes.append(float(data[4]) + float(data[6]) + float(data[8]))
    else:
        rTimes.append(float(data[3]) + float(data[6]) + float(data[8]))
rTimes.sort()

for i in range(0,len(rTimes)):
    print(str(i) + "," + str(rTimes[i]))
