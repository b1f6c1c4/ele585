#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <cmath>
#include <iomanip>

template <typename ... TArgs>
void write_log(size_t nmach, size_t my, TArgs ... args)
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	std::cout << std::put_time(std::localtime(&in_time_t), "%FT%T ");
	std::cout << "[" << std::setfill('0') << std::setw(std::log10(nmach)+1) << my << "] ";
	(std::cout << ... << args);
	std::cout << std::endl;
}
