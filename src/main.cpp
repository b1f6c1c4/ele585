#include <iostream>
#include <algorithm>
#include <cstdint>
#include "quick_sort.hpp"
#include "timed.hpp"

#ifndef GRP
#define GRP 1024
#endif

int main(int argc, char *argv[])
{
    if (argc != 2)
        return 1;

    constexpr auto grp = GRP;
    auto len = std::atoi(argv[1]);
    auto buffer = new uint64_t[len];

    std::cin.read(reinterpret_cast<char *>(buffer), sizeof(*buffer) * len);
    {
        timed t{};
        for (auto ptr = buffer; ptr < buffer + len; ptr += grp)
#ifndef STDSORT
            quick_sort(ptr, ptr + grp);
#else
            std::sort(ptr, ptr + grp);
#endif
    }
    std::cout.write(reinterpret_cast<const char *>(buffer), sizeof(*buffer) * len);

    return 0;
}
