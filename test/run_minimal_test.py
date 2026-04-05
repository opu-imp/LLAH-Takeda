#!/usr/bin/env python3
"""Minimal test: just run llahdoc with one query, check if it finishes."""

import subprocess
import os
import time

BASE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(BASE)
LLAHDOC = os.path.join(ROOT, "llahdoc")
DB_DATA = os.path.join(BASE, "db_data2")
QUERY_DIR = os.path.join(BASE, "query_images")
RESULT = os.path.join(BASE, "minimal_result.txt")

with open(RESULT, "w") as f:
    q = os.path.join(QUERY_DIR, "query_doc005.jpg")
    cmd = [LLAHDOC, "-h", DB_DATA + "/", q]
    f.write(f"CMD: {' '.join(cmd)}\n")
    f.write(f"START: {time.strftime('%H:%M:%S')}\n")
    f.flush()

    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=ROOT)
    try:
        stdout, stderr = proc.communicate(timeout=60)
        f.write(f"END: {time.strftime('%H:%M:%S')}\n")
        f.write(f"EXIT: {proc.returncode}\n")
        f.write(f"STDOUT:\n{stdout.decode('utf-8', errors='replace')}\n")
        f.write(f"STDERR:\n{stderr.decode('utf-8', errors='replace')}\n")
    except subprocess.TimeoutExpired:
        proc.kill()
        stdout, stderr = proc.communicate()
        f.write(f"TIMEOUT at {time.strftime('%H:%M:%S')}\n")
        f.write(f"STDOUT so far:\n{stdout.decode('utf-8', errors='replace')}\n")
        f.write(f"STDERR so far:\n{stderr.decode('utf-8', errors='replace')}\n")

print("Done")
