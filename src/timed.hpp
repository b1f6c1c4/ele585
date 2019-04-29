#pragma once

#include <iostream>
#include <chrono>
#include <atomic>

class timed final
{
public:
	timed() : _start(std::chrono::high_resolution_clock::now()) { }

	timed(const timed &) = delete;
	timed(timed &&) = delete;
	timed &operator=(const timed &) = delete;
	timed &operator=(timed &&) = delete;

	auto operator()() const
	{
		auto stop = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::nanoseconds>(
				stop - _start
				).count();
	}

private:
	std::chrono::high_resolution_clock::time_point _start;
};

class aggregated final
{
public:
	aggregated() : _sum(0) { }

	aggregated(const aggregated &) = delete;
	aggregated(aggregated &&) = delete;
	aggregated &operator=(const aggregated &) = delete;
	aggregated &operator=(aggregated &&) = delete;

	class guard final
	{
	public:
		guard(aggregated *aggr) : _aggr(aggr), _start(std::chrono::high_resolution_clock::now()) { }

		guard(const guard &) = delete;
		guard(guard &&other) : _aggr(other._aggr), _start(other._start)
		{
			other._aggr = nullptr;
		}

		guard &operator=(const guard &) = delete;
		guard &operator=(guard &&other)
		{
			if (this == &other)
				return *this;

			save();
			_aggr = other._aggr;
			_start = other._start;
		}

		~guard()
		{
			save();
		}

	private:
		void save()
		{
			if (_aggr)
			{
				_aggr->_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(
						std::chrono::high_resolution_clock::now() - _start
						).count();
				_aggr = nullptr;
			}
		}

		aggregated *_aggr;
		std::chrono::high_resolution_clock::time_point _start;
	};

	auto fork() { return guard{this}; }

	auto operator()() const volatile noexcept { return _sum.load(); }

private:
	volatile std::atomic<uint64_t> _sum;
};
