#!/bin/python

#print(prefix + ",benchmark,solve mem,solve time,drat kb,drat sec,lrat kb,lrat sec,restarts,decisions,conflicts,propagations,mark proof sec,dump lrat sec, ana sec, anamem mb")

        
        
fAna = open("ana.csv", "r")
rTimes = list()
rDict = dict()
for l in fAna:
    data = l.split(",")
    rDict.update({data[2]:(float(data[4]), float(data[8]))})

fStd = open("std.csv", "r")
for l in fStd:
    data = l.split(",")
    sTime = float(data[3])
    if rDict[data[2]][0] < sTime:
        print("wrong: " + data[2] + "t: " + str(sTime) + " to " + str(rDict[data[2]]))
        assert False
    
    aTime = rDict[data[2]][0]-sTime
    lTime = rDict[data[2]][1]
    
    rTimes.append((sTime,aTime,lTime))
        

rTimes.sort(key=lambda tup: tup[0])

for i in range(0,len(rTimes)):
    print(str(i+1) + "," + str(rTimes[i][0]) + "," + str(rTimes[i][1]) + "," + str(rTimes[i][2]))
