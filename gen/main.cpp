#include <iostream>
#include "generator.h"

class cxx_generator : protected sorting_network_generator
{
protected:
    virtual void sort_unit(size_t l, size_t r)
    {
        if (_count++ % 6 == 0)
            std::cout << std::endl << "       ";
        std::cout << " X(" << l << ", " << r << ");";
    }

    size_t _count;
public:
    void generate(size_t sz)
    {
        _count = 0;
        std::cout << std::endl;
        std::cout << "    case " << sz << ":" << std::endl;
        sorting_network_generator::sort(sz);
        std::cout << std::endl;
        std::cout << "        break; // " << _count << " units" << std::endl;
    }
};

class graph_generator : protected sorting_network_generator
{
protected:
    virtual void sort_unit(size_t l, size_t r)
    {
        for (size_t i = 0; i < l; i++)
            std::cout << " | ";
        std::cout << " X-";
        for (size_t i = l + 1; i < r; i++)
            std::cout << "-+-";
        std::cout << "-X ";
        for (size_t i = r + 1; i < _sz; i++)
            std::cout << " | ";
        std::cout << std::endl;
    }

    size_t _sz;
public:
    void generate(size_t sz)
    {
        _sz = sz;
        sorting_network_generator::sort(sz);
    }
};

int main(int argc, char *argv[])
{
    auto kind = false;
    auto max = 128;

    for (auto i = 1; i < argc; i++)
        if (std::string(argv[i]) == "--graph")
            kind = true;
        else
            max = std::atoi(argv[i]);

    if (kind)
    {
        graph_generator{}.generate(max);
        return 0;
    }

    cxx_generator gen{};

    std::cout << R"(#pragma once

#include <algorithm>

#define X(l, r) if (d[r] < d[l]) std::swap(l, r)

template <typename Iter>
inline void sn_sort(Iter first, Iter last)
{
    if (last <= first)
        return;

    switch (last - first)
    {
)";

    for (auto i = 0; i <= max; i++)
        gen.generate(i);

    std::cout << R"(
    default:

        std::sort(first, last);
        break;
    }
}
)";
}
