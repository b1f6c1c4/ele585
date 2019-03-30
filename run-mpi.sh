#!/bin/sh

X="$1"
NT="$2"
N="$3"
MX="$4"

if [ "$X" = "H" ]; then
    BIN="./bin/parallel_mpi/F"
else
    BIN="./bin/parallel_mpi/$X"
fi
IN="./data/input/$N.txt"
if [ "$MX" -eq "255" ]; then
    STD="./data/standard/balanced/$N.txt"
else
    STD="./data/standard/unbalanced/$N.txt"
fi

WKDIR="/scratch/jinzheng/ele585/$X-$NT-$N-$MX"
T="$WKDIR/tmp.txt"
mkdir -p "$WKDIR"

I=0
while [ "$I" -lt "500" ]; do
    srun -n "$NT" "$BIN" "$IN" "$N" "$MX" 1 > "$T"
    RET="$?"
    if [ "$RET" -ne "0" ]; then
        cat "$T"
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
