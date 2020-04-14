#!/bin/sh

set -uxo pipefail

GRP="$1"
LENG="$2"
FILE="$3"

T="/scratch/jinzheng/ele585-bmark/$$"

mkdir -p "$T"
cp -R src/ "$T/"

cd "$T"

mkdir -p bin/

g++ -std=c++17 -O3 -Wall -Werror -Wextra \
    -DGRP="$1" \
    -o bin/sn-mpi-sn src/main.cpp

g++ -std=c++17 -O3 -Wall -Werror -Wextra \
    -DGRP="$1" -DSTDSORT \
    -o bin/sn-mpi-std src/main.cpp

RES_SN="$(bin/sn-mpi-sn "$LENG" <"$FILE" 2>&1 >/dev/null)"
if [ "$?" -ne "0" ]; then
    echo "$RES_SN"
    exit 1;
fi
RES_STD="$(bin/sn-mpi-std "$LENG" <"$FILE" 2>&1 >/dev/null)"
if [ "$?" -ne "0" ]; then
    echo "$RES_STD"
    exit 1;
fi

python -c "print 1.0 - $RES_SN.0 / $RES_STD.0"

rm -rf "$T"
