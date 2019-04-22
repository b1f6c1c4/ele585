#include <iostream>
#include <algorithm>
#include <cstdint>
#include "finite_sort.hpp"
#include "main.h"

int main(int argc, char *argv[])
{
    constexpr auto grp = 8;
    constexpr auto len = grp; // 1024 * 1024 * 1024;
    auto buffer = new uint64_t[len];

    std::cin.read(reinterpret_cast<char *>(buffer), sizeof(*buffer) * len);

    for (auto ptr = buffer; ptr < buffer + len; ptr += grp)
        finite_sort<uint64_t, grp>().sort(ptr);
        // std::sort(ptr, ptr + grp);

    std::cout.write(reinterpret_cast<const char *>(buffer), sizeof(*buffer) * len);

    return 0;
}
