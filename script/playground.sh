#!/bin/bash

set -euo pipefail

make -j10

./bin/sn-mpi >./bin/log

ARGS=()
for I in $(seq 0 3); do
    grep "^$I " ./bin/log | awk '{printf "%-32s\n", $0}' > "./bin/log-$I"
    ARGS+=("./bin/log-$I")
done

paste "${ARGS[@]}"
grep '^Final' ./bin/log

rm -rf ./bin/log-*
