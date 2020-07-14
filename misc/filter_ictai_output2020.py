import sys

elapsedStr = "Elapsed (wall clock) time (h:mm:ss or m:ss):"
memUsedStr = "Maximum resident set size (kbytes):"

def getSecFromTimeVerbose(inStr):
#Elapsed (wall clock) time (h:mm:ss or m:ss): 8:26.39
    assert (elapsedStr in inStr)
    numberStr = inStr[inStr.find(elapsedStr) + len(elapsedStr):].replace(" ","")
    numberVec = numberStr.split(":")
    
    if len(numberVec) == 3:
        hours = float(numberVec[0])
        minuits = float(numberVec[1])
        seconds = float(numberVec[2])
        return ((hours * 60.0) + minuits) * 60.0 + seconds
        
    else:
        assert len(numberVec) == 2
        minuits = float(numberVec[0])
        seconds = float(numberVec[1])
        return minuits * 60 + round(seconds)

def getKbFromTimeVerbose(inStr):
#Elapsed (wall clock) time (h:mm:ss or m:ss): 8:26.39
    assert (memUsedStr in inStr)
    numberStr = inStr[inStr.find(memUsedStr) + len(memUsedStr):].replace(" ","")
    return int(numberStr)
    

def getKBytesFromLinuxDu(inStr):
    #1770868	/local/proof.lrat
    strVec = inStr.split('\t')
    if len(strVec) == 1:
        strVec = inStr.split(' ')
    return int(strVec[0])
    
def getValFromSATComment(inStr):
    #c propagations          : 254450008      (544763 /sec),
    #Marking proof: 6.92 seconds,
    #c Dumping LRAT: 25.68 seconds,
    assert inStr.count(':') > 0
    strVec = inStr.split(':')[1]
    #find start . end
    s = 0
    for s in range(0,len(strVec)):
        if strVec[s].isdigit():
            break
    e = s
    for e in range(s,len(strVec)):
        if not strVec[e].isdigit():
            break
    if len(strVec) > e+1 and strVec[e] == '.': # case for float
        e2 = e
        for e2 in range(e+1,len(strVec)):
            if not strVec[e2].isdigit():
                break
        return float(strVec[s:e2])
    else:
        return int(strVec[s:e])
    
def getSatFile(inStr):
    #/nfs/scratch/bzchartu/unsat_2018_solved/ae_rphp035_05.cnf,
    assert ".cnf" in inStr
    inStr = inStr.split('/')[-1]
    return inStr[0:inStr.find(".cnf")] + ".cnf"

def cleanCsvLine(inStr, prefix):
    res = str(prefix) + ","
    strVec = inStr.split(',')
    for s in strVec:
        if ".cnf" in s:
            res += getSatFile(s)
        elif "OVERVIEW_CSV" in s:
            continue
        elif elapsedStr in s:
            res += str(getSecFromTimeVerbose(s))
        elif memUsedStr in s:
            res += str(getKbFromTimeVerbose(s))
        elif ":" in s:
            res += str(getValFromSATComment(s))
        elif len(s) > 0 and s[0].isdigit():
            res += str(getKBytesFromLinuxDu(s))
        else:
            res += s
        res += ","
    return res[0:len(res)-1]

assert len(sys.argv) > 1

f = open(sys.argv[1], "r")
prefix = sys.argv[1]
counter = 1
#print(prefix + ",id,benchmark,solve mem,solve time,drat kb,drat sec,lrat kb,lrat sec,restarts,decisions,conflicts,propagations,mark proof sec,dump lrat sec, ana sec, anamem mb")
for l in f:
    if l.count(",") > 0:
        print(cleanCsvLine(l,prefix+","+str(counter)))
        counter += 1
