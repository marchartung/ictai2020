#!/bin/python

#print(prefix + ",benchmark,solve mem,solve time,drat kb,drat sec,lrat kb,lrat sec,restarts,decisions,conflicts,propagations,mark proof sec,dump lrat sec, ana sec, anamem mb")

memQuot = 1000*1000

fAna = open("ana.csv", "r")
rTimes = list()
rDict = dict()
for l in fAna:
    data = l.split(",")
    rDict.update({data[2]:(float(data[4]), float(data[8]), float(data[16]), float(data[7]))})

fStd = open("std.csv", "r")

print("bench & solve & time drat & time lrat & solve & time lrat & TTFV (improvement) & &drat & lrat & online disk memory & lrat & completeMem (overhead)\\\\")

for l in fStd:
    data = l.split(",")
    sTime = int(float(data[3]))
    dratTime = int(float(data[6]))
    dlratTime = int(float(data[8]))
    
    sLTime = int(rDict[data[2]][0])
    lTime = int(rDict[data[2]][1])
    
    ttfv = sLTime+lTime
    impr = round(((float(ttfv) /(sTime+dratTime))-1)*100.0,1)
    
    dratMem = round(float(data[5])/memQuot,1)
    dlratMem = round(float(data[7])/memQuot,1)
    
    solveMem = round(rDict[data[2]][2]/memQuot*1000.0,1)
    lratMem = round(rDict[data[2]][3]/memQuot,1)
    completeMem = solveMem + lratMem
    overhe = round(100.0*completeMem/dratMem,1)
    
    
    print(data[2] + " & " + str(sTime) + " & " + str(dratTime) + " & " + str(dlratTime)  + " & " + str(sLTime) + " & " + str(lTime) + " & " + str(ttfv) + " (" + str(impr) + "\\%) & & " + str(dratMem) + " & " + str(dlratMem) + " & " + str(solveMem) + " & " + str(lratMem) + " & " + str(completeMem) +  " (" + str(overhe) + "\\%)\\\\")
    
