#!/usr/bin/env python3
"""Run llahdoc query tests and write results."""

import subprocess
import os

BASE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(BASE)
LLAHDOC = os.path.join(ROOT, "llahdoc")
DB_DATA = os.path.join(BASE, "db_data")
QUERY_DIR = os.path.join(BASE, "query_images")

queries = [
    "query_doc001.jpg",
    "query_doc003.jpg",
    "query_doc005.jpg",
]

results = []
for q in queries:
    qpath = os.path.join(QUERY_DIR, q)
    cmd = [LLAHDOC, "-g", "53", "5", "15", "-h", DB_DATA + "/", qpath]
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=30, cwd=ROOT)
        results.append(f"=== {q} ===\nSTDOUT:\n{r.stdout}\nSTDERR:\n{r.stderr}\nEXIT: {r.returncode}\n")
    except subprocess.TimeoutExpired:
        results.append(f"=== {q} ===\nTIMEOUT after 30s\n")
    except Exception as e:
        results.append(f"=== {q} ===\nERROR: {e}\n")

outpath = os.path.join(BASE, "test_results.txt")
with open(outpath, "w") as f:
    f.write("\n".join(results))
print(f"Results written to {outpath}")
