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

    if (argc < 4)
        return 3;

    const size_t nmem = std::atoll(argv[1]);
    const size_t nmsg = std::atoll(argv[3]);

    int nmach, my;
    MPI_Comm_size(MPI_COMM_WORLD, &nmach);
    MPI_Comm_rank(MPI_COMM_WORLD, &my);

    if (argc != 4 && argc != nmach + 3)
    {
        std::cerr
            << nmach << " machines, "
            << argc - 3 << " + 3 argc" << std::endl;
        return 3;
    }

    const auto fid = argc == 4 ? 3 : my + 3;

    std::cerr
        << "Mach #" << my << "/" << nmach
        << " operating on " << argv[fid]
        << " with NMem=" << nmem << std::endl;

    auto buffer = new size_t[nmem];

    bitonic_remote_mpi<size_t> sorter(nmach, nmem, nmsg, argv[fid], buffer);
    sorter.execute(my);

    delete [] buffer;
    buffer = nullptr;

    MPI_Finalize();
}
