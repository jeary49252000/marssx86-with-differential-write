#!/usr/bin/python
import os
import re
import math
import sys

if len(sys.argv) != 3:
    print "\n usage: script end_insts(M) directory\n"
    sys.exit(0)

 
END_INSTS =    sys.argv[1] #
DIR = sys.argv[2]

NUM_INSTS = int(END_INSTS)

Suffix=["w160.r80.b16",
        "w160.r40.b16",
        "w160.r160.b16",
        "w160.r80.b8",
        "w160.r80.b32",
        "w80.r80.b16",
        "w320.r80.b16"]

Benchmark=["410.bwaves", "434.zeusmp", "437.leslie3d", "470.lbm", "473.astar"]

#Benchmark=["410.bwaves", "429.mcf", "434.zeusmp", "436.cactusADM", "459.GemsFDTD", "470.lbm", "473.astar", "stream", "mummer", "mix1", "mix2", "mix3"]

DRAMresults=[
#"total read count",  "total read latency", 
#"total write count", "total write latency",
"total refresh count", "total refresh latency",
#"request accept cnt", "request reject cnt",
"Marss total write count", "Marss total write latency for wait", "Marss total write latency for process",
"Marss total read count", "Marss total read latency for wait", "Marss total read latency for process" ]

results = {}


def findResult(string, pattern):
    return int(re.search(pattern + " : " + "(\d+)", string).group(1))

# =======================
for B in Benchmark:
    for S in Suffix:
        File = DIR + B + '.' + S + '.log'
        if os.path.exists(File):
            with open(File, "r") as inp:
                wholefile = inp.read()
                


                match = re.search("Completed +(\d+) cycles, +(" + END_INSTS + "\d\d\d\d\d\d" + ") commits", wholefile)
                
                if match:
                    results[("end", "cycles", B, S)] = int(match.group(1))
                    
                    slicefile = wholefile[match.start():]
                    for D in DRAMresults:
                        results["end", D, B, S] = findResult(slicefile, D)


print "\nIPC"
for B in Benchmark:
    sb = B + ", "
    for S in Suffix:
        if results.has_key(("end", "cycles", B, S)):
            sb += str(round(NUM_INSTS * 1000000.0 / (results[("end", "cycles", B, S)]), 3))
        sb += ", "
    print sb



print "\nexecution time (cpu cycles)"
for B in Benchmark:
    sb = B + ", "
    for S in Suffix:
        if results.has_key(("end", "cycles", B, S)):
            sb += str(results[("end", "cycles", B, S)] )
        sb += ", "
    print sb


for D in DRAMresults:
    print "\n" + D
    for B in Benchmark:
        sb = B + ", "
        for S in Suffix:
            if results.has_key(("end", D, B, S)):
                sb += str( results[("end", D, B, S)] )
            sb += ", "
        print sb

print "\nIPC"
for B in Benchmark:
    sb = B + ", "
    for S in Suffix:
        if results.has_key(("end", "cycles", B, S)):
            sb += str(round(NUM_INSTS * 1000000.0 / (results[("end", "cycles", B, S)]), 3))
        sb += ", "
    print sb








