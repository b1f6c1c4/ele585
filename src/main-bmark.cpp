#include <iostream>
#include <algorithm>
#include <cstdint>
#include <climits>
#include <mpi.h>
#include <experimental/filesystem>
#include "logger.hpp"
#include "fast_random.hpp"
#include "bitonic_mpi.hpp"
#include "timed.hpp"
#include "check_ordering.hpp"

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

#define SHOW_TIME(str, v) \
	({ \
		decltype(auto) res = v; \
		LOG(str, " time = ", res, "ns = ", res / 60e9, "min"); \
	})

namespace fs = std::experimental::filesystem;

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    if (argc != 3)
	{
		std::cout
			<< "Usage: mpiexec -n <NMach> bin/sn-mpi-bmark \\" << std::endl
			<< "    <NMem> <NMsg>" << std::endl;
        return 3;
	}

    const size_t nmem = std::atoll(argv[1]);
    const size_t nmsg = std::atoll(argv[2]);

    int nmach, my;
    MPI_Comm_size(MPI_COMM_WORLD, &nmach);
    MPI_Comm_rank(MPI_COMM_WORLD, &my);
#define LOG(...) write_log(nmach, my, __VA_ARGS__)

	fs::path ftmp;

	LOG("NMem=", nmem, " NMsg=", nmsg);

	auto buffer = new size_t[nmem];

	fast_random rnd(114514 + nmem + nmsg + nmach + my);
	rnd(buffer, nmem);
	LOG("In-memory generation finished");

	MPI_Barrier(MPI_COMM_WORLD);

	timed t{};
	{
		LOG("In-memory sorting started");
		bitonic_remote_mpi<size_t> sorter(nmach, nmem, nmsg, buffer);
		sorter.execute(my);
		SHOW_TIME("Local computation", sorter._comp());
		SHOW_TIME("Local communication", sorter._comm());
	}

	SHOW_TIME("Local total", t());

	MPI_Barrier(MPI_COMM_WORLD);
	SHOW_TIME("Global total", t());

	LOG("Checking started");
	auto ret = 0;
	{
		if (!check_ordering<size_t>(nmach, my, check_ordering(buffer, nmem)))
			ret = 1;
		delete [] buffer;
		buffer = nullptr;
	}
	if (ret)
		LOG("Global result: incorrect");
	else
		LOG("Global result: correct");

    MPI_Finalize();
	return ret;
}
