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

namespace fs = std::experimental::filesystem;

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    if (argc != 5)
	{
		std::cout
			<< "Usage: mpiexec -n <NMach> bin/sn-mpi-bmark \\" << std::endl
			<< "    <NMem> <NSec> <NMsg> <tmpdir>" << std::endl;
        return 3;
	}

    const size_t nmem = std::atoll(argv[1]);
    const size_t nsec = std::atoll(argv[2]);
    const size_t nmsg = std::atoll(argv[3]);

    int nmach, my;
    MPI_Comm_size(MPI_COMM_WORLD, &nmach);
    MPI_Comm_rank(MPI_COMM_WORLD, &my);
#define LOG(...) write_log(nmach, my, __VA_ARGS__)

	fs::path ftmp = argv[4];
	fs::create_directories(ftmp);
	ftmp /= std::to_string(my);

	LOG("NMem=", nmem, " NSec=", nsec, " NMsg=", nmsg);
	LOG("operating on ", ftmp);

	fast_random rnd(114514);
	{
		std::ofstream f(ftmp, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
		const auto sz = std::min(static_cast<size_t>(8 * 1024), nmem * nsec);
		size_t buffer[sz];
		for (size_t i = 0; i < nmem * nsec; i += sz)
		{
			rnd(buffer, sz);
			if (!f.write(reinterpret_cast<const char *>(buffer), sz * sizeof(size_t)))
				throw std::runtime_error("Can't write file");
		}
	}
	LOG("generation finished");

	MPI_Barrier(MPI_COMM_WORLD);
	LOG("sorting started");

	{
		timed t{};
		bitonic_remote_mpi<size_t> sorter(nmach, nmem, nsec, nmsg, ftmp);
		sorter.execute(my);
		const auto res = t.done();
		LOG("Total time = ", res, "ns = ", res / 60e9, "min");
	}

	MPI_Barrier(MPI_COMM_WORLD);
	LOG("checking started");

	auto ret = 0;
	{
		std::ifstream f(ftmp, std::ios_base::in | std::ios_base::binary);
		if (!f.is_open())
			throw std::runtime_error("Can't open file");
		if (!check_ordering<size_t>(nmach, my, f))
		{
			LOG("The result is incorrect");
			ret = 1;
		}
		else
			LOG("The result is correct");
	}

	if (!fs::remove(ftmp))
		LOG("Warning: can't remove temp file");

    MPI_Finalize();
	return ret;
}
