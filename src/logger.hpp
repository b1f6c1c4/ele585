#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <cmath>
#include <iomanip>

#ifdef __INTEL_COMPILER
// Intel icc is silly.

template <typename T>
void write_log(T first)
{
	std::cout << first;
}

template <typename T, typename ... TArgs>
void write_log(T first, TArgs ... rest)
{
	std::cout << first;
	write_log(rest...);
}
#endif

template <typename ... TArgs>
void write_log(size_t nmach, size_t my, TArgs ... args)
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	std::cout << std::put_time(std::localtime(&in_time_t), "%FT%T ");
	std::cout << "[" << std::setfill('0') << std::setw(std::log10(nmach)+1) << my << "] ";
#ifndef __INTEL_COMPILER
	(std::cout << ... << args);
#else
	write_log(args...);
#endif

	std::cout << std::endl;
}
