#!/bin/python


fAna = open("ana.csv", "r")
rTimes = list()
rDict = dict()
#print(prefix + ",benchmark,solve mem,solve time,drat kb,drat sec,lrat kb,lrat sec,restarts,decisions,conflicts,propagations,mark proof sec,dump lrat sec, ana sec, anamem mb")

for l in fAna:
    data = l.split(",")
    rDict.update({data[2]:float(data[4])})

fStd = open("std.csv", "r")

for l in fStd:
    data = l.split(",")
    sTime = float(data[3])
    dTime = float(data[6])
    print(str(rDict[data[2]]-sTime) + "," + str(dTime))
