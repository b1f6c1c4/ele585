#!/bin/sh

FOLDER="$1"
N="$2"
SIZE="$3"
BS="$4"

mkdir -p "$FOLDER"

for I in $(seq 0 $(($N-1))); do
	if [ -f "$FOLDER/$I" ] && [ "$(stat --printf='%s' "$FOLDER/$I")" -eq "$(($SIZE*$BS))" ]; then
		echo "Skip $FOLDER/$I"
	else
		dd if=/dev/urandom of="$FOLDER/$I" count="$SIZE" bs="$BS"
	fi
done
