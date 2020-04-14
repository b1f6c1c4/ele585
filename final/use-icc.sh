if [ "$_" = "$0" ]; then
    echo "Nope, you should source this"
    exit 1
fi

if [ "$SHELL" != "/bin/bash" ]; then
    echo "Nope, you MUST use /bin/bash, not $SHELL"
fi

module load intel openmpi

export CXX=icc

module list
