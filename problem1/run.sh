#!/bin/sh

X="$1"
NT="$2"
N="$3"
MX="$4"

BIN="./bin/parallel/$X"
IN="./data/input/$N.txt"
if [ "$MX" -eq "255" ]; then
    STD="./data/standard/balanced/$N.txt"
else
    STD="./data/standard/unbalanced/$N.txt"
fi

WKDIR="/scratch/jinzheng/ele585/$X-$NT-$N-$MX"
T="$WKDIR/tmp.txt"
mkdir -p "$WKDIR"

if [ "$N" -le "128" ]; then
    ITER=$((1024 / $NT))
elif [ "$N" -le "8192" ]; then
    ITER=500
elif [ "$N" -le "524288" ]; then
    ITER=100
else # if [ "$N" -le "33554432" ]; then
    ITER=50
fi

I=0
while [ "$I" -lt "$ITER" ]; do
    "$BIN" "$IN" "$N" "$MX" "$NT" > "$T"
    RET="$?"
    if [ "$RET" -ne "0" ]; then
        cat "$T"
        echo ">> Pass $I: Error $RET"
        exit "$RET"
    fi
    sed '$ d' "$T" | cmp --silent - "$STD" >/dev/null
    if [ "$?" -eq "0" ]; then
        echo ">> Pass $I: Good"
    else
        echo ">> Pass $I: Fail"
        cat "$T"
        rm -f "$T"
        rmdir "$WKDIR" || true
        exit 1
    fi
    sed -n '$ p' "$T"
    I=$(($I+1))
done

rm -f "$T"
rmdir "$WKDIR" || true
