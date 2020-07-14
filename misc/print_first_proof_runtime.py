#!/bin/python

import sys

def eprint(stri):
    print(stri, file=sys.stderr)   

assert len(sys.argv) > 1

f = open(sys.argv[1], "r")
prefix = sys.argv[1]
rTimes = list()
#print(prefix + ",benchmark,solve mem,solve time,drat kb,drat sec,lrat kb,lrat sec,restarts,decisions,conflicts,propagations,mark proof sec,dump lrat sec, ana sec, anamem mb")

spRatio = 0.0

for l in f:
    ratio = 0.0
    data = l.split(",")
    if len(data) > 15 :
        rTimes.append(float(data[4]) + float(data[8]))
        ratio = float(data[4])/( float(data[4]) +float(data[8]))
        
    else:
        rTimes.append(float(data[3]) + float(data[6]))
        ratio = float(data[3])/( float(data[3]) +float(data[6]))
        
    spRatio += ratio
rTimes.sort()

for i in range(0,len(rTimes)):
    print(str(i) + "," + str(rTimes[i]/3600))

eprint( prefix + " : "+ str(spRatio/len(rTimes))) 
