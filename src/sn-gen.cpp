#include <config.h>
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
        for (size_t i = l; i < r - 1; i++)
            std::cout << "-+-";
        std::cout << "-X ";
        for (size_t i = r; i < _sz; i++)
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

    for (size_t i = 0; i <= 128; i++)
        gen.generate(i);

    std::cout << R"(
    default:
        std::sort(first, last);
        break;
    }
}
)";
}
