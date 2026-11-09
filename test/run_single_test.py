#!/usr/bin/env python3
"""Run a single llahdoc query test with longer timeout."""

import subprocess
import os

BASE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(BASE)
LLAHDOC = os.path.join(ROOT, "llahdoc")
DB_DATA = os.path.join(BASE, "db_data")
QUERY_DIR = os.path.join(BASE, "query_images")

q = "query_doc005.jpg"
qpath = os.path.join(QUERY_DIR, q)
cmd = [LLAHDOC, "-g", "53", "5", "15", "-h", DB_DATA + "/", qpath]

outpath = os.path.join(BASE, "single_result.txt")
with open(outpath, "w") as f:
    f.write(f"CMD: {' '.join(cmd)}\n")
    f.write("Starting...\n")
    f.flush()
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=120, cwd=ROOT)
        f.write(f"STDOUT:\n{r.stdout}\n")
        f.write(f"STDERR:\n{r.stderr}\n")
        f.write(f"EXIT: {r.returncode}\n")
    except subprocess.TimeoutExpired:
        f.write("TIMEOUT after 120s\n")
    except Exception as e:
        f.write(f"ERROR: {e}\n")

print(f"Done. Results in {outpath}")
