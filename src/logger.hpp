#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

template <typename ... TArgs>
void write_log(TArgs ... args)
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	std::cout << std::put_time(std::localtime(&in_time_t), "%FT%T ");
	(std::cout << ... << args);
	std::cout << std::endl;
}
