#!/usr/bin/env python3
import sys
from pathlib import Path

LIMIT = int(sys.argv[-1])
TARGETS = [Path(p) for p in sys.argv[1:-1]]

def iter_files(targets):
    for target in targets:
        if target.is_file() and target.suffix in [".c", ".h"]:
            yield target
        elif target.is_dir():
            for p in target.rglob("*"):
                if p.suffix in [".c", ".h"]:
                    yield p

def scan_file(path: Path):
    with path.open("r", encoding="utf-8", errors="ignore") as f:
        lines = f.readlines()

    in_func = False
    brace_depth = 0
    start_line = 0
    violations = []

    for i, line in enumerate(lines, start=1):
        stripped = line.strip()

        if not in_func:
            if "(" in stripped and ")" in stripped and "if" not in stripped and "for" not in stripped and "while" not in stripped and "switch" not in stripped:
                if stripped.endswith("{"):
                    in_func = True
                    start_line = i
                    brace_depth = line.count("{") - line.count("}")
        else:
            brace_depth += line.count("{") - line.count("}")
            if brace_depth == 0:
                length = i - start_line + 1
                if length > LIMIT:
                    violations.append((start_line, i, length))
                in_func = False

    return violations

failed = False

for file in iter_files(TARGETS):
    violations = scan_file(file)
    for start, end, length in violations:
        failed = True
        print(f"[FAIL] {file}: lines {start}-{end}, function length={length} > {LIMIT}")

if failed:
    sys.exit(1)

print("[PASS] All functions satisfy line limit.")
