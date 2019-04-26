#include <iostream>
#include <algorithm>
#include <cstdint>
#include <climits>
#include <mpi.h>
#include "bitonic_mpi.hpp"

#if SIZE_MAX == UCHAR_MAX
#define MPI_SIZE_T MPI_UNSIGNED_CHAR
#elif SIZE_MAX == USHRT_MAX
#define MPI_SIZE_T MPI_UNSIGNED_SHORT
#elif SIZE_MAX == UINT_MAX
#define MPI_SIZE_T MPI_UNSIGNED
#elif SIZE_MAX == ULONG_MAX
#define MPI_SIZE_T MPI_UNSIGNED_LONG
#elif SIZE_MAX == ULLONG_MAX
#define MPI_SIZE_T MPI_UNSIGNED_LONG_LONG
#endif

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    if (argc != 4)
        return 1;

    const size_t nmem = std::atoi(argv[1]);
    const size_t nsec = std::atoi(argv[2]);

    int nmach, my;
    MPI_Comm_size(MPI_COMM_WORLD, &nmach);
    MPI_Comm_rank(MPI_COMM_WORLD, &my);

    bitonic_remote_mpi<size_t> sorter(nmach, nmem, nsec, argv[3]);
    sorter.execute(my);

    MPI_Finalize();
}
