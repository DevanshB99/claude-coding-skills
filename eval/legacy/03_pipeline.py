import os, sys, json, csv

def run():
    cfg = json.load(open("config.json"))
    data = []
    for fn in os.listdir(cfg["indir"]):
        if fn.endswith(".csv"):
            for row in csv.DictReader(open(os.path.join(cfg["indir"], fn))):
                data.append(row)
    clean = []
    for d in data:
        ok = True
        for k in ["id", "value", "date"]:
            if k not in d or d[k] == "":
                ok = False
        if ok:
            try:
                d["value"] = float(d["value"])
            except:
                ok = False
        if ok: clean.append(d)
    tot = {}
    for c in clean:
        m = c["date"][0:7]
        tot[m] = tot.get(m, 0) + c["value"]
    o = open(cfg["outdir"] + "/summary.csv", "w")
    o.write("month,total\n")
    for m in sorted(tot): o.write(m + "," + str(tot[m]) + "\n")
    o.close()

run()
