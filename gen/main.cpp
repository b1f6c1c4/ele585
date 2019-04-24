#include <string>
#include <iostream>
#include "generator.h"

class cxx_generator : protected sorting_network_generator
{
protected:
    virtual void sort_unit(size_t l, size_t r)
    {
        if (_count++ % 6 == 0)
            std::cout << std::endl << "   ";
        std::cout << " X(" << l << ", " << r << ");";
    }

    size_t _count;
public:
    void generate(size_t sz, const std::string &type)
    {
        _count = 0;
        std::cout << std::endl
            << "void sn_sort_" << sz << "(" << type << " *d)" << std::endl
            << "{";
        sorting_network_generator::sort(sz);
        std::cout << std::endl
            << "} // " << _count << " units" << std::endl;
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
    auto id = 0;
    auto type = std::string("size_t");
    std::vector<size_t> szs;

    for (auto i = 1; i < argc; i++)
        if (std::string(argv[i]) == "--graph")
            id = 1, kind = true;
        else if (id == 0)
            id = 1, type = argv[i];
        else
            id = 1, szs.push_back(std::atoi(argv[i]));

    if (kind)
    {
        for (auto sz : szs)
            graph_generator{}.generate(sz);
        return 0;
    }

    cxx_generator gen{};

    std::cout << R"(#include "sn.h"
#include <utility>

#define X(l, r) if (d[r] < d[l]) std::swap(d[l], d[r])
)";

    for (auto sz : szs)
        gen.generate(sz, type);

    std::cout << std::endl;
    std::cout << "void sn_sort(" << type << " *first, " << type << " *last)";
    std::cout << R"(
{
    if (last <= first)
        return;

    switch (last - first)
    {
        case 0:
        case 1:
            return;
)";

    for (auto sz : szs)
        std::cout << "        case " << sz << ": sn_sort_" << sz << "(first); break;" << std::endl;

    std::cout << R"(
        default:
            throw;
    }
}
)";
}
