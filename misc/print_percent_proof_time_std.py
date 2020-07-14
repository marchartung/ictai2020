#!/bin/python

import sys

assert len(sys.argv) > 1

f = open(sys.argv[1], "r")
prefix = sys.argv[1]
rTimes = list()
#print(prefix + ",benchmark,solve mem,solve time,drat kb,drat sec,lrat kb,lrat sec,restarts,decisions,conflicts,propagations,mark proof sec,dump lrat sec, ana sec, anamem mb")

for l in f:
    
    data = l.split(",")
    rTimes.append((float(data[3]), float(data[6]), float(data[8])))
rTimes.sort(key=lambda tup: tup[0])

for i in range(0,len(rTimes)):
    print(str(i+1) + "," + str(rTimes[i][0]) + "," + str(rTimes[i][1]) + "," + str(rTimes[i][2]))
