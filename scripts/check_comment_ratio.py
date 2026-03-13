#!/usr/bin/env python3
import sys
from pathlib import Path

MIN_RATIO = float(sys.argv[-1])
TARGETS = [Path(p) for p in sys.argv[1:-1]]

def iter_files(targets):
    for target in targets:
        if target.is_file() and target.suffix in [".c", ".h"]:
            yield target
        elif target.is_dir():
            for p in target.rglob("*"):
                if p.suffix in [".c", ".h"]:
                    yield p

comment_lines = 0
code_lines = 0
in_block = False

for file in iter_files(TARGETS):
    with file.open("r", encoding="utf-8", errors="ignore") as f:
        for raw in f:
            line = raw.strip()
            if not line:
                continue

            if in_block:
                comment_lines += 1
                if "*/" in line:
                    in_block = False
                continue

            if line.startswith("//"):
                comment_lines += 1
            elif line.startswith("/*"):
                comment_lines += 1
                if "*/" not in line:
                    in_block = True
            else:
                code_lines += 1

total = comment_lines + code_lines
ratio = (comment_lines / total * 100.0) if total else 0.0

print(f"comment_lines={comment_lines}")
print(f"code_lines={code_lines}")
print(f"comment_ratio={ratio:.2f}%")

if ratio < MIN_RATIO:
    print(f"[FAIL] Comment ratio {ratio:.2f}% < {MIN_RATIO}%")
    sys.exit(1)

print("[PASS] Comment ratio check passed.")
