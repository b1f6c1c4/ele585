#!/bin/sh

set -euo pipefail

if [ "$#" -eq "0" ]; then
    find data -maxdepth 1 -type f -name '*G-*.log' | xargs -r -n 1 "$0"
    exit 0
fi

PREFIX="$(sed -r 's_^.*/([0-9]+)G-([0-9]+)\.log$_\1,\2_' <<<"$1")"

grep 'Global total time' "$1" | grep -Po '([.0-9]+)(?=min)' | sed 's_^.*$_'"$PREFIX"',total,\0_'
grep 'Local computation time' "$1" | grep -Po '([.0-9]+)(?=min)' | sed 's_^.*$_'"$PREFIX"',computation,\0_'
grep 'Local communication time' "$1" | grep -Po '([.0-9]+)(?=min)' | sed 's_^.*$_'"$PREFIX"',communication,\0_'
