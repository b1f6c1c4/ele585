#!/bin/sh

find data -maxdepth 1 -type f -name '*G-*.log' | \
    xargs -n 1 bash -c 'printf "%s," "$1" && grep -Po "[.\d]+(?=min)" "$1" | head -n 1' '' | \
    sed 's_data/__; s_G-_,_; s_x.*\.log__' | \
    tee ./data/report.csv
