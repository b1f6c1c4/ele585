#pragma once

#include <cstddef>
#include <iostream>
#include <optional>
#include <functional>
#include <limits>
#include <mpi.h>

#define LOG(...) write_log(nmach, my, __VA_ARGS__)

template <typename T>
std::optional<std::pair<T, T>> check_ordering(const T *d, size_t sz, bool reversed = false)
{
	if (sz < 2)
		return { std::make_pair(d[0], d[sz - 1]) };

	auto last = *d;
	while (--sz)
	{
		auto the = *++d;
		if (reversed ? (last < the) : (the < last))
			return {};
		last = the;
	}
	return { std::make_pair(d[0], last) };
}

template <typename T, size_t Blk = 8 * 1024>
std::optional<std::pair<T, T>> check_ordering(std::istream &f)
{
	T buffer[Blk];

	std::optional<T> begin;
	std::optional<T> end;

	f.seekg(0, f.end);
	size_t fsz = f.tellg();
	f.seekg(0, f.beg);

	while (fsz)
	{
		auto rd = std::min(fsz, sizeof(T) * Blk);
		if (!f.read(reinterpret_cast<char *>(buffer), rd))
			throw std::runtime_error("Can't raed file");

		if (!begin)
			begin = { buffer[0] };

		decltype(auto) res = check_ordering(buffer, rd / sizeof(T));
		if (!res)
			return {};

		end = { res.value().second };

		fsz -= rd;
	}

	if (begin && end)
		return { std::make_pair(begin.value(), end.value()) };

	return {};
}

template <typename T, size_t Blk = 8 * 1024>
bool check_ordering(size_t nmach, size_t my, const std::optional<std::pair<T, T>> &res)
{
	auto local_result = !!res;
	bool global_result;

	LOG("Local result: ", local_result ? "correct" : "incorrect");
	MPI_Allreduce(
			&local_result, &global_result, 1,
			MPI_CXX_BOOL, MPI_LAND, MPI_COMM_WORLD);

	if (!global_result)
		return false;

	T buffer;
	const auto buffer_ptr = reinterpret_cast<void *>(&buffer);
	for (size_t p = 0; p < std::log2(nmach); p++)
	{
        const auto mask = static_cast<size_t>(1) << p;
		const auto partner = my ^ mask;
		const int tag = std::numeric_limits<int>::max() - p;
		LOG("Checking level ", p + 1);
		if ((my & mask) == 0)
		{
			buffer = res.value().second;
			MPI_Send(
					buffer_ptr, sizeof(T),
					MPI_UNSIGNED_CHAR, partner, tag, MPI_COMM_WORLD);
			break;
		}
		MPI_Recv(
				buffer_ptr, sizeof(T),
				MPI_UNSIGNED_CHAR, partner, tag, MPI_COMM_WORLD, nullptr);
		if (res.value().first < buffer)
		{
			local_result = false;
			break;
		}
	}

	LOG("Local merge result: ", local_result ? "correct" : "incorrect");
	MPI_Allreduce(
			&local_result, &global_result, 1,
			MPI_CXX_BOOL, MPI_LAND, MPI_COMM_WORLD);

	return global_result;
}

#undef LOG
