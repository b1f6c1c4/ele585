#!/bin/sh

set -euxo pipefail

F0="$1"
N="$2"

SZ="$(stat --printf="%s" "$F0")"
BS=$(($SZ/$N))
if [ "$BS" -gt 8192 ]; then
    BS=8192
fi

for I in $(seq 0 $(($N-1))); do
	dd if="$F0" of="$F0-$I" skip=$(($SZ/$N/$BS*$I)) count=$(($SZ/$N/$BS)) bs=$BS
done
