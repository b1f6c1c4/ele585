#!/bin/bash

set -euo pipefail

# Usage:
#
#     To test performance using /tmp/ele585-bmark,
#         using 16 processes within 1 hour,
#         each with 8192MiB main buffer & 32MiB peer buffer,
#         each file of size 128GiB:
#
#     sbatch -n 16 --mem-per-cpu 8500 -t 1:00:00 \
#         script/bmark.sh 8192 32 /tmp/ele585-bmark $((1024 * 128))
#
#     Note:
#         The folder does NOT need to pre-exist.

printf '+ script/bmark.sh' 1>&2
printf ' %q' "$@" 1>&2
echo '' 1>&2

MEM="$1"
shift
MSG="$1"
shift
DIR="$1"
shift
SEC="$1"
shift

# Number of size_t entries in main buffer, single shard
NMEM="$(($MEM * 1024 * 1024 / 8))"
# Number of size_t entries in peer buffer, single shard
NMSG="$(($MSG * 1024 * 1024 / 8))"
# Number of sections, single shard
NSEC="$(($SEC / $MEM))"

set -x

mpirun ./bin/sn-mpi-bmark "$NMEM" "$NSEC" "$NMSG" "$DIR"
