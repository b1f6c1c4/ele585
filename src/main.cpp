#include <iostream>
#include <algorithm>
#include <cstdint>
#include "finite_sort.hpp"
#include "main.h"

int main(int argc, char *argv[])
{
    constexpr auto len = 1024 * 1024 * 1024;
    constexpr auto grp = 8;
    auto buffer = new uint64_t[len];

    std::cin.read(reinterpret_cast<char *>(buffer), sizeof(*buffer) * len);

    for (auto ptr = buffer; ptr < buffer + len; ptr += grp)
        std::sort(ptr, ptr + grp);
        // finite_sort<uint64_t, grp>().sort(ptr);

    std::cout.write(reinterpret_cast<const char *>(buffer), sizeof(*buffer) * len);

    return 0;
}
