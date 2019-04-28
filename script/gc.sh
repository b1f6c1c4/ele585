#!/bin/bash

set -euo pipefail

usage()
{
    cat - <<EOF
Usage:
    ./script/gc.sh
        [-d <directory>]
EOF
}

DIR=/tmp/ele585-bmark
while [ "$#" -gt "0" ]; do
    case "$1" in
        -h|--help)
            usage
            exit 2
            ;;
        -d|--directory)
            DIR="$2"
            shift
            shift
            ;;
    esac
done

sinfo -N -o '%n' | xargs -n 1 -P 0 -r -t \
    bash -c 'ssh "$2" rm -rf "$1" || true' '' "$DIR"
