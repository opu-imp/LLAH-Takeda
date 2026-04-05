#!/usr/bin/env python3
"""Full test: rebuild DB with small hash, query, and report results."""

import subprocess
import os
import shutil

BASE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(BASE)
LLAHDOC = os.path.join(ROOT, "llahdoc")
DB_IMG = os.path.join(BASE, "db_images")
DB_DATA = os.path.join(BASE, "db_data2")
QUERY_DIR = os.path.join(BASE, "query_images")
RESULT = os.path.join(BASE, "full_test_results.txt")

# Clean db_data2
if os.path.exists(DB_DATA):
    shutil.rmtree(DB_DATA)
os.makedirs(DB_DATA)

results = []

# Step 1: Build DB
results.append("=== STEP 1: DB Construction ===")
cmd = [LLAHDOC, "-c", DB_IMG + "/", "jpg", "-h", DB_DATA + "/"]
try:
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=60, cwd=ROOT)
    results.append(f"EXIT: {r.returncode}")
    results.append("STDERR (last 10 lines):\n" + "\n".join(r.stderr.strip().split("\n")[-10:]))
except subprocess.TimeoutExpired:
    results.append("TIMEOUT (60s)")
except Exception as e:
    results.append(f"ERROR: {e}")

# Check generated files
results.append("\n=== DB Files ===")
if os.path.exists(DB_DATA):
    for f in sorted(os.listdir(DB_DATA)):
        fp = os.path.join(DB_DATA, f)
        results.append(f"  {f}: {os.path.getsize(fp)} bytes")

# Step 1.5: Generate point_comp.dat
results.append("\n=== STEP 1.5: Generate point_comp.dat ===")
cmd = [LLAHDOC, "-f", "-h", DB_DATA + "/"]
try:
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=30, cwd=ROOT)
    results.append(f"EXIT: {r.returncode}")
    results.append(f"STDERR:\n{r.stderr.strip()}")
except subprocess.TimeoutExpired:
    results.append("TIMEOUT (30s)")
except Exception as e:
    results.append(f"ERROR: {e}")

# Read config.dat
results.append("\n=== config.dat ===")
cfg_path = os.path.join(DB_DATA, "config.dat")
if os.path.exists(cfg_path):
    with open(cfg_path) as f:
        results.append(f.read().strip())

# Step 2: Query tests
queries = ["query_doc001.jpg", "query_doc003.jpg", "query_doc005.jpg"]
for q in queries:
    results.append(f"\n=== STEP 2: Query {q} ===")
    qpath = os.path.join(QUERY_DIR, q)
    cmd = [LLAHDOC, "-h", DB_DATA + "/", qpath]
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=120, cwd=ROOT)
        results.append(f"EXIT: {r.returncode}")
        stdout_lines = r.stdout.strip().split("\n") if r.stdout.strip() else ["(empty)"]
        results.append("STDOUT:\n" + "\n".join(stdout_lines))
        stderr_lines = r.stderr.strip().split("\n") if r.stderr.strip() else ["(empty)"]
        results.append("STDERR:\n" + "\n".join(stderr_lines[-5:]))
    except subprocess.TimeoutExpired:
        results.append("TIMEOUT (120s)")
    except Exception as e:
        results.append(f"ERROR: {e}")

# Write results
with open(RESULT, "w") as f:
    f.write("\n".join(results) + "\n")

print(f"Done. Results in {RESULT}")
