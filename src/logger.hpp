#pragma once

#include <iostream>

template <typename ... TArgs>
void write_log(TArgs ... args)
{
	(std::cerr << ... << args);
	std::cerr << std::endl;
}
