#!/bin/bash
#
#SBATCH --job-name=ele585
#SBATCH --output=./data/mpi.out
#
#SBATCH --ntasks=4
#SBATCH --mem-per-cpu=160
#SBATCH --time=1:00:00

set -euo pipefail

F0=/tigress/jinzheng/input-short

FILES=()
for I in $(seq 0 $(($SLURM_NPROCS-1))); do
    FILES+=("$F0-$I")
done

echo "All data should have been stored on" "${FILES[@]}"

# Number of bytes, total input
SZ="$(stat --printf="%s" "$F0")"
# Number of size_t entries in main buffer, single shard
NMEM="$((128 * 1024 * 1024 / 8))" # 128 MiB
# Number of size_t entries in peer buffer, single shard
NMSG="$((32 * 1024 * 1024 / 8))" # 32 MiB
if [ "$((8 * $SLURM_NPROCS * $NMEM))" -gt "$SZ" ]; then
    NMEM="$(($SZ / 8 / $SLURM_NPROCS))"
fi
# Number of sections, single shard
NSEC="$(($SZ / 8 / $SLURM_NPROCS / $NMEM))"

echo "Sort started at $(date -Ins)"
mpirun ./bin/sn-mpi "$NMEM" "$NSEC" "$NMSG" "${FILES[@]}"
echo "Sort completed at $(date -Ins)"

echo "All data has been sorted and written to" "${FILES[@]}"
