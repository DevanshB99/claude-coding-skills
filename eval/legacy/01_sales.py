import csv, os, sys

def load_and_calculate(file, thresh=0, dbg=False):
    d = {}
    try:
        f = open(file, 'r')
        r = csv.reader(f)
        hdr = next(r)
        for row in r:
            try:
                if len(row) < 3: continue
                reg = row[1]
                rev = float(row[2])
                if rev > thresh:
                    if reg in d: d[reg] = d[reg] + rev
                    else: d[reg] = rev
                if dbg: print("processed " + reg)
            except:
                pass
        f.close()
    except Exception as e:
        print("error " + str(e))
        return None
    tot = 0
    for k in d: tot = tot + d[k]
    if tot > 1000000:
        print("WARNING: total exceeds 1000000")
    return d
