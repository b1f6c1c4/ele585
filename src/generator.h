#pragma once

#include <cstddef>

#define RNG(x) size_t N##x, size_t NS##x, size_t B##x, size_t NT##x
#define RNX(x) N##x, NS##x, B##x, NT##x

class generator
{
protected:
    virtual void sort_unit(size_t l, size_t r) = 0;

private:
    static constexpr size_t ceil_2(size_t v) { return v - (v / 2); }
    static constexpr size_t floor_2(size_t v) { return v / 2; }

    void sorting_network(RNG());
    void odd_even(RNG(a), RNG(b));

public:
    void sort(size_t sz) { sorting_network(sz, 1, 0, sz); }
};
