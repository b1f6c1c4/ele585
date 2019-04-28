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

template <typename T>
class memory_stream : public std::istream
{
public:
	memory_stream(const T *d, size_t sz)
		: std::istream(&_b), _b(d, sz)
	{
		rdbuf(&_b);
	}

private:
	struct buffer_t : public std::basic_streambuf<char>
	{
		buffer_t(const T *d, size_t sz)
		{
			auto ptr = const_cast<char *>(reinterpret_cast<const char *>(d));
			setg(ptr, ptr, ptr + sz * sizeof(T));
		}
	};

	buffer_t _b;
};

namespace fs = std::experimental::filesystem;

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    if (argc != 4 && argc != 5)
	{
		std::cout
			<< "Usage: mpiexec -n <NMach> bin/sn-mpi-bmark \\" << std::endl
			<< "    <NMem> <NSec> <NMsg> [<tmpdir>]" << std::endl;
        return 3;
	}

    const size_t nmem = std::atoll(argv[1]);
    const size_t nsec = std::atoll(argv[2]);
    const size_t nmsg = std::atoll(argv[3]);

    int nmach, my;
    MPI_Comm_size(MPI_COMM_WORLD, &nmach);
    MPI_Comm_rank(MPI_COMM_WORLD, &my);
#define LOG(...) write_log(nmach, my, __VA_ARGS__)

	fs::path ftmp;
	if (nsec > 1)
	{
		ftmp = argv[4];
		fs::create_directories(ftmp);
		ftmp /= std::to_string(my);
	}

	LOG("NMem=", nmem, " NSec=", nsec, " NMsg=", nmsg);
	if (nsec > 1)
		LOG("operating on ", ftmp);

	auto buffer = new size_t[nmem];

	fast_random rnd(114514 + nmem + nsec + nmsg + nmach + my);
	if (nsec > 1)
	{
		std::ofstream f(ftmp, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
		for (size_t i = 0; i < nsec; i++)
		{
			rnd(buffer, nmem);
			if (!f.write(reinterpret_cast<const char *>(buffer), nmem * sizeof(size_t)))
				throw std::runtime_error("Can't write file");
		}
		LOG("Generation finished");
	}
	else
	{
		rnd(buffer, nmem);
		LOG("In-memory generation finished");
	}

	MPI_Barrier(MPI_COMM_WORLD);

	timed t{};
	if (nsec > 1)
	{
		LOG("Sorting started");
		bitonic_remote_mpi<size_t> sorter(nmach, nmem, nsec, nmsg, ftmp, buffer);
		sorter.execute(my);
	}
	else
	{
		LOG("In-memory sorting started");
		bitonic_remote_mpi<size_t> sorter(nmach, nmem, nmsg, buffer);
		sorter.execute(my);
	}

	{
		const auto res = t.done();
		LOG("Local total time = ", res, "ns = ", res / 60e9, "min");
	}

	MPI_Barrier(MPI_COMM_WORLD);
	{
		const auto res = t.done();
		LOG("Global total time = ", res, "ns = ", res / 60e9, "min");
	}

	delete [] buffer;
	buffer = nullptr;

	LOG("Checking started");
	auto ret = 0;
	if (nsec == 1)
	{
		memory_stream<size_t> f(buffer, nmem);
		if (!check_ordering<size_t>(nmach, my, f))
		{
			LOG("Global result: incorrect");
			ret = 1;
		}
		else
			LOG("Global result: correct");
	}
	else
	{
		std::ifstream f(ftmp, std::ios_base::in | std::ios_base::binary);
		if (!f.is_open())
			throw std::runtime_error("Can't open file");
		if (!check_ordering<size_t>(nmach, my, f))
		{
			LOG("Global result: incorrect");
			ret = 1;
		}
		else
			LOG("Global result: correct");
	}

	if (!fs::remove(ftmp))
		LOG("Warning: can't remove temp file");

    MPI_Finalize();
	return ret;
}
