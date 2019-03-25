#!/bin/sh

X="$1"
NT="$2"
N="$3"

BIN="./bin/parallel/$X"
IN="./data/input/$N.txt"
STD="./data/standard/$N.txt"

WKDIR="/scratch/jinzheng/ele585/$X-$NT-$N"
T="$WKDIR/tmp.txt"
mkdir -p "$WKDIR"

I=0
while [ "$I" -lt "10" ]; do
    "$BIN" "$IN" "$N" 255 "$NT" > "$T"
    RET="$?"
    if [ "$RET" -ne "0" ]; then
        echo ">> Pass $I: Error $RET"
        exit "$RET"
    fi
    sed '$ d' "$T" | cmp --silent - "$STD" >/dev/null
    if [ "$?" -eq "0" ]; then
        echo ">> Pass $I: Good" 1>&2
    else
        echo ">> Pass $I: Fail" 1>&2
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
