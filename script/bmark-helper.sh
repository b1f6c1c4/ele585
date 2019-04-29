#!/bin/bash

set -euo pipefail

usage()
{
    cat - <<EOF
Usage:
    ./script/bmark-helper.sh
        [-t <time>]
        [-m <message size (MiB)>]
        <total data (GiB)>
        <number of parallel tasks>
        [-- <...other sbatch parameters>]
EOF
}

TIME=24:00:00
MSG=32
POSITIONAL=()
EXTRA=()
while [ "$#" -gt "0" ]; do
    case "$1" in
        -h|--help)
            usage
            exit 2
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

make -j8 ./bin/sn-mpi-bmark

mkdir -p data

EXTRA+=(--output "data/${SZ0}G-${N}x$(($MEM/1024))G.log")
EXTRA+=(--job-name "${SZ0}/${N}x$(($MEM/1024))G")
EXTRA+=(--ntasks "$N")
EXTRA+=(--cpus-per-task 1)
EXTRA+=(--mem-per-cpu "$MEMORY")
EXTRA+=(--time "$TIME")

sbatch \
    "${EXTRA[@]}" \
    ./script/bmark.sh "$MEM" "$MSG"

echo "You may want to less +F data/${SZ0}G-${N}x$(($MEM/1024))G.log"
