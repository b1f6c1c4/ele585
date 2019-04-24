#include <config.h>
#include <iostream>
#include "generator.h"

class cxx_generator : protected generator
{
protected:
    virtual void sort_unit(size_t l, size_t r)
    {
        if (_count++ % 8 == 0)
            std::cout << std::endl << "   ";
        std::cout << " X(" << l << ", " << r << ");";
    }

    size_t _count;
public:
    void generate(size_t sz)
    {
        _count = 0;
        std::cout << "template <typename T>" << std::endl;
        std::cout << "void finite_sort<T, " << sz << ">(T *d)" << std::endl;
        std::cout << "{";
        generator::sort(sz);
        std::cout << std::endl << "} // " << _count << " units" << std::endl;
    }
};

int main(int argc, char *argv[])
{
    cxx_generator gen{};

    std::cout << R"(
#include <algorithm>

#define X(l, r) if (d[r] < d[l]) std::swap(l, r)

template <typename T, size_t N>
void finite_sort<T, N>(T *d)
{
    std::sort(d, d + N);
}
)";

    for (size_t i = 0; i < 100; i++)
        gen.generate(i);
}
