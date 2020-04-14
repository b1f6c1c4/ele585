if [ "$_" = "$0" ]; then
    echo "Nope, you should source this"
    exit 1
fi

module load rh/devtoolset/7 openmpi/gcc/2.0.2/64

export CXX=g++

module list
