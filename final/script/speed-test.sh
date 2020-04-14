#!/bin/bash

dd if=/dev/zero of=$1 count=$((1024 * 1024)) bs=8192 status=progress

rm -f $1
