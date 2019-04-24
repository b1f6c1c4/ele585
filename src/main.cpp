#include <iostream>
#include <algorithm>
#include <cstdint>
#include "sn.hpp"
#include "timed.hpp"

int main(int argc, char *argv[])
{
    if (argc != 3)
        return 1;

    auto grp = std::atoi(argv[1]);
    auto len = std::atoi(argv[2]);
    auto buffer = new uint64_t[len];

    std::cin.seekg(0, std::cin.beg);
    std::cin.read(reinterpret_cast<char *>(buffer), sizeof(*buffer) * len);
    {
        timed t{};
        for (auto ptr = buffer; ptr < buffer + len; ptr += grp)
            sn_sort(ptr, ptr + grp);
    }
    std::cout.write(reinterpret_cast<const char *>(buffer), sizeof(*buffer) * len);

    return 0;
}
