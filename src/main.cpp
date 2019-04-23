#include <iostream>
#include <algorithm>
#include <cstdint>
#include "timed.hpp"
#include "finite_sort.hpp"
#include "main.h"

int main(int argc, char *argv[])
{
    constexpr auto grp = 19;
    auto len = grp; // std::atoi(argv[1]);
    auto buffer = new uint64_t[len];

    std::cin.seekg(0, std::cin.beg);
    std::cin.read(reinterpret_cast<char *>(buffer), sizeof(*buffer) * len);
    {
        timed t{};
        for (auto ptr = buffer; ptr < buffer + len; ptr += grp)
            std::sort(ptr, ptr + grp);
    }
    std::cout.write(reinterpret_cast<const char *>(buffer), sizeof(*buffer) * len);

    std::cin.seekg(0, std::cin.beg);
    std::cin.read(reinterpret_cast<char *>(buffer), sizeof(*buffer) * len);
    {
        timed t{};
        for (auto ptr = buffer; ptr < buffer + len; ptr += grp)
            finite_sort<uint64_t, grp>().sort(ptr);
    }
    std::cout.write(reinterpret_cast<const char *>(buffer), sizeof(*buffer) * len);

    return 0;
}
