#!/bin/python

import sys

def eprint(stri):
    print(stri, file=sys.stderr)    

fAna = open("ana.csv", "r")
rTimes = list()
rDict = dict()
#print(prefix + ",benchmark,solve mem,solve time,drat kb,drat sec,lrat kb,lrat sec,restarts,decisions,conflicts,propagations,mark proof sec,dump lrat sec, ana sec, anamem mb")

for l in fAna:
    data = l.split(",")
    rDict.update({data[2]:(float(data[16]),float(data[7]))})

fStd = open("std.csv", "r")

smaller = 0
larger = 0

counter = 0
sumOverhead = 0.0
sumOverheadLrat = 0.0



counter = 1
for l in fStd:
    data = l.split(",")
    dratU = float(data[5])/1000
    onlineMem = rDict[data[2]][0]
    onlineLrat = rDict[data[2]][1]/1000
    print(str(counter) + "," + str(onlineMem+onlineLrat) + "," + str(dratU+float(data[7])/1000))
    
    if onlineMem + onlineLrat <= 10*dratU:
        smaller += 1
    else:
        larger += 1
        
    sumOverhead += (onlineMem + onlineLrat)/dratU
    sumOverheadLrat += (onlineMem + onlineLrat)/(dratU+float(data[7])/1000)
    counter += 1
 

#eprint("Memory overhead:")
#eprint("average: " + str(round(sumOverhead/counter,2)))
#eprint("average with lrat: " + str(round(sumOverheadLrat/counter,2)))
#eprint("smaller 10x: " + str(smaller))
#eprint("larger 10x: " + str(larger))
