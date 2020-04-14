#!/bin/bash

set -euo pipefail

usage()
{
    cat - <<EOF
Usage:
    ./script/bmark-helper.sh [-a]
        [-t <time>] [-o <output>]
        [-m <message size (MiB)>]
        <total data (GiB)>
        <number of parallel tasks>
        [-- <...other sbatch parameters>
        [-- <...other mpirun parameters>]]
EOF
}

TIME=24:00:00
MSG=32
ATTACH=
OUTPUT=
POSITIONAL=()
EXTRA=()
while [ "$#" -gt "0" ]; do
    case "$1" in
        -h|--help)
            usage
            exit 2
            ;;
        -a|--attach)
            ATTACH=YES
            shift
            ;;
        -t|--time)
            TIME="$2"
            shift
            shift
            ;;
        -m|--message-size)
            MSG="$2"
            shift
            shift
            ;;
        -o|--output)
            OUTPUT="$2"
            shift
            shift
            ;;
        --)
            shift
            break
            ;;
        *)
            POSITIONAL+=("$1")
            shift
            ;;
    esac
done

while [ "$#" -gt "0" ]; do
    if [ "$1" = "--" ]; then
        shift
        break
    fi
    EXTRA+=("$1")
    shift
done

if [ "${#POSITIONAL[@]}" -ne "2" ]; then
    usage
    exit 2
fi

SZ0="${POSITIONAL[0]}"
N="${POSITIONAL[1]}"
MEM="$((1024 * $SZ0 / $N))"
MEMORY="$(($MEM + 512 + $MSG))"
if [ -z "$OUTPUT" ]; then
    OUTPUT="data/${SZ0}G-${N}.log"
fi

make -j8 ./bin/sn-mpi-bmark

mkdir -p data

EXTRA+=(--job-name "${SZ0}/${N}")
EXTRA+=(--ntasks "$N")
EXTRA+=(--cpus-per-task 1)
EXTRA+=(--mem-per-cpu "$MEMORY")
EXTRA+=(--time "$TIME")

if [ ! -z "$ATTACH" ]; then
    salloc \
        "${EXTRA[@]}" \
        ./script/bmark.sh "$MEM" "$MSG" "$@" \
        >"$OUTPUT"
else
    sbatch \
        --output "$OUTPUT" \
        "${EXTRA[@]}" \
        ./script/bmark.sh "$MEM" "$MSG" "$@"

    echo "You may want to less +F data/${SZ0}G-${N}.log"
fi
