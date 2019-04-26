#!/bin/bash
#
#SBATCH --job-name=ele585
#SBATCH --output=./data/mpi.out

set -euo pipefail

# Usage:
#
#     To sort file(s) /tigress/jinzheng/input-*,
#         using 16 processes within 1 hour,
#         each with 8192MiB main buffer & 32MiB peer buffer:
#
#     sbatch -n 16 --mem 8500 -t 1:00:00 \
#         script/mpi.sh 8192 32 /tigress/jinzheng/input-*
#
#     Note:
#         If only 1 file is specified, then it will be splitted;
#         otherwise, -n must match the number of files.

MEM="$1"
shift
MSG="$1"
shift

if [ "$#" -eq "$SLURM_NPROCS" ]; then
    XSZ=
    FILES=()
    while [ "$#" -gt "0" ]; do
        SZ="$(stat --printf="%s" "$1")"
        if [ ! -z "$XSZ" ] && [ "$SZ" -ne "$XSZ" ]; then
            echo "File sizes not equal"
            exit 2
        fi
        XSZ="$SZ"
        FILES+=("$1")
        shift
    done
elif [ "$#" -eq "1" ]; then
    F0="$1"

    FILES=()
    for I in $(seq 0 $(($SLURM_NPROCS-1))); do
        FILES+=("$F0-$I")
    done

    echo "File partition started at $(date -Ins)"
    ./script/split-file.sh "$F0" "$SLURM_NPROCS"

    SZ="$(stat --printf="%s" "$F0")"
    XSZ="$(($SZ / $SLURM_NPROCS))"
else
    echo "File numbers not match"
    exit 2
fi

# Number of size_t entries in main buffer, single shard
NMEM="$(($MEM * 1024 * 1024 / 8))"
# Number of size_t entries in peer buffer, single shard
NMSG="$(($MSG * 1024 * 1024 / 8))"
if [ "$((8 * $NMEM))" -gt "$XSZ" ]; then
    NMEM="$(($XSZ / 8))"
fi
# Number of sections, single shard
NSEC="$(($XSZ / 8))"

echo "Sort started at $(date -Ins)"
/usr/bin/time --verbose mpirun ./bin/sn-mpi "$NMEM" "$NSEC" "$NMSG" "${FILES[@]}"
echo "Sort completed at $(date -Ins)"

printf '%s\n' "All data has been sorted and written to" "${FILES[@]}"
