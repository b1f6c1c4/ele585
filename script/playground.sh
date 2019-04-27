#!/bin/bash

set -euo pipefail

make -j10 bin/sn-playground

mkdir -p data

./bin/sn-playground >./data/log

ARGS=()
for I in $(seq 0 1); do
    grep "^$I " ./data/log | awk '{printf "%-64s\n", $0}' > "./data/log-$I"
    ARGS+=("./data/log-$I")
done

paste "${ARGS[@]}"
grep '^Final' ./data/log

rm -rf ./data/log-*
