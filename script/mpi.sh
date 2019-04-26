#!/bin/bash
#
#SBATCH --job-name=ele585
#SBATCH --output=mpi.out
#
#SBATCH --ntasks=4
#SBATCH --mem-per-cpu=160
#SBATCH --time=1:00:00

set -euxo pipefail

F0=/tigress/jinzheng/input-short
F1=/tigress/jinzheng/output-short
FTMP=/scratch/jinzheng/ele585-mpi

SZ="$(stat --printf="%s" "$F0")"

BS=8192
COUNT="$(($SZ / $BS))"
SKIP="$(($COUNT / $SLURM_NPROCS))"

NMEM="$((128 * 1024 * 1024 / 8))" # 128 MiB
NSEC="$(($BS * $SKIP / 8 / $NMEM))"

if [ "$NSEC" -lt "1" ]; then
    NMEM="$(($BS * $SKIP / 8))"
    NSEC=1
fi

DEBUG=
# DEBUG="echo "

rm -f "$F1"

F0Q="$(printf '%q' "$F0")"
F1Q="$(printf '%q' "$F1")"
FTMPDQ="$(printf '%q' "$FTMP/")"
FTMPQ="$(printf '"$(printf '"'"'%%s/%%s'"'"' %q \$SLURM_PROCID.dat)"' "$FTMP")"
SHIFTQ="\$((\$SLURM_PROCID * $SKIP))"

srun bash -c "$DEBUG mkdir -p $FTMPDQ && $DEBUG rm -f $FTMPQ"
srun bash -c "$DEBUG dd if=$F0Q of=$FTMPQ skip=$SHIFTQ count=$SKIP bs=$BS"
srun bash -c "$DEBUG mpiexec bin/sn-mpi $NMEM $NSEC $FTMPQ"
srun bash -c "$DEBUG dd if=$FTMPQ of=$F1Q seek=$SHIFTQ count=$SKIP bs=$BS"
