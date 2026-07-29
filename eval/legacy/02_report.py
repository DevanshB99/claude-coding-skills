import datetime, sqlite3

CONN = sqlite3.connect("/var/data/sales.db")

def make_report():
    cur = CONN.cursor()
    cur.execute("SELECT region, revenue FROM sales WHERE year = " + str(datetime.date.today().year))
    rows = cur.fetchall()
    out = []
    for r in rows:
        if r[1] != None and r[1] > 0:
            if r[0] != None:
                out.append((r[0], r[1] * 1.2))
    f = open("/var/reports/out.txt", "w")
    for o in out:
        f.write(o[0] + "," + str(o[1]) + "\n")
    f.close()
    print("wrote " + str(len(out)) + " rows")
    return len(out)
