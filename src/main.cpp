#include <iostream>
#include <algorithm>
#include <cstdint>
#include "finite_sort.hpp"
#include "main.h"

int main(int argc, char *argv[])
{
    constexpr auto len = 5; // 1 * 1024 * 1024;
    auto buffer = new uint64_t[len];

    std::cin.read(reinterpret_cast<char *>(buffer), sizeof(*buffer) * len);

    finite_sort<uint64_t, len>(buffer);

    std::cout.write(reinterpret_cast<const char *>(buffer), sizeof(*buffer) * len);

    return 0;
}
