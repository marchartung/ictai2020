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
    rTimes.append(((rDict[data[2]]-float(data[3]))/(rDict[data[2]]+float(data[3])),(rDict[data[2]]-float(data[3]))/(rDict[data[2]]+float(data[3]))))
    #print("B:" + data[2] + ": " + str(rDict[data[2]]) + " - " + str(float(data[3])) + " = " + str(rDict[data[2]]-float(data[3])))
rTimes.sort()
    
for i in range(0,len(rTimes)):
    print(str(i) + "," + str(rTimes[i][0]) + "," + str(rTimes[i][1]))
