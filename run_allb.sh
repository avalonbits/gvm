#!/bin/bash
for b in $(\find ./perf -type f | sort)
  do
    echo ""
    echo "BENCH=$b ./run_bench.sh"
    BENCH=$b ./run_bench.sh
  done
