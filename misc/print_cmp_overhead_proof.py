#!/bin/python

#print(prefix + ",benchmark,solve mem,solve time,drat kb,drat sec,lrat kb,lrat sec,restarts,decisions,conflicts,propagations,mark proof sec,dump lrat sec, ana sec, anamem mb")

  
import sys

def eprint(stri):
    print(stri, file=sys.stderr)      
        
fAna = open("ana.csv", "r")
rTimes = list()
rDict = dict()
for l in fAna:
    data = l.split(",")
    rDict.update({data[2]:(float(data[4]), float(data[8]), float(data[13]))})

overallTreduction = 0.0
sumDrat = 0.0
sumPtb = 0.0

fStd = open("std.csv", "r")
for l in fStd:
    data = l.split(",")
    sTime = float(data[3])
    dratTime = float(data[6])
    if rDict[data[2]][0] < sTime:
        eprint("wrong: " + data[2] + "t: " + str(sTime) + " to " + str(rDict[data[2]]))
        assert False
    
    dumpTime = 0
    aTime = rDict[data[2]][0]-sTime
    lTime = rDict[data[2]][1]
    dTime = float(data[6])
    sumDrat += sTime+dratTime
    sumPtb += rDict[data[2]][0] + lTime
    overallTreduction += (rDict[data[2]][0] + lTime)/(sTime+dratTime)
        
    rTimes.append((sTime,aTime,dumpTime,lTime,dTime))

rTimes.sort(key=lambda tup: tup[4])
for i in range(0,len(rTimes)):
    print(str(i+1) + "," + str(rTimes[i][0]) + "," + str(rTimes[i][1]) + "," + str(rTimes[i][2]) + "," + str(rTimes[i][3]) + "," + str(rTimes[i][4]))

tRed = round(100.0*(1.0-overallTreduction/len(rTimes)),1) 
tOverall = round(100.0*(1.0-sumPtb/sumDrat),1) 

eprint("average time reduction: " + str(tRed) + "%")
eprint("overall time reduction: " + str(tOverall) + "%")
