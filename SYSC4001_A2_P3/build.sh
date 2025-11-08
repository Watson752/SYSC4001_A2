#!/usr/bin/env bash
set -euo pipefail

echo "[1/3] Compiling..."
g++ -std=gnu++17 -O0 -g -Wall -Wextra -o interrupts interrupts.cpp

echo "[2/3] Preflight checks..."
for f in trace.txt vector_table.txt device_table.txt external_files.txt; do
  [[ -f "input_files/$f" ]] || { echo "Missing input_files/$f"; exit 1; }
done
echo "All inputs present."

echo "[3/3] Running simulator from inside input_files/..."
(
  cd input_files
  BIN=../interrupts
  [[ -f ../interrupts.exe ]] && BIN=../interrupts.exe
  "$BIN" trace.txt vector_table.txt device_table.txt external_files.txt
)

echo "Done. Outputs are in input_files/:"
ls -l input_files/execution.txt input_files/system_status.txt
