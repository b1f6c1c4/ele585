#pragma once

#include <cstddef>
#include <queue>
#include <array>
#include <utility>
#include <iostream>

#define RNG(x) size_t N##x, size_t NS##x, size_t B##x, size_t NT##x
#define RNX(x) N##x, NS##x, B##x, NT##x

template <typename T>
class generator_t
{
public:
    bool eof() const { return _q.empty(); }
    T next()
    {
        decltype(auto) v = _q.front();
        _q.pop();
        return v;
    }

    void push(T &&v) { _q.push(std::move(v)); }

    template <typename ... TArgs>
    void merge(TArgs &&... others)
    {
        std::array<generator_t<T>, sizeof...(TArgs)> arr = {{ others... }};
        auto flag = true;
        while (flag)
        {
            flag = false;
            for (auto &g : arr)
                if (!g.eof())
                    flag = true, push(std::move(g.next()));
        }
    }

private:
    std::queue<T> _q;
};

class sorting_network_generator
{
protected:
    virtual void sort_unit(size_t l, size_t r) = 0;

private:
    typedef std::pair<size_t, size_t> pair_t;
    typedef generator_t<pair_t> gen_t;

    static constexpr size_t ceil_2(size_t v) { return v - (v / 2); }
    static constexpr size_t floor_2(size_t v) { return v / 2; }

    gen_t sorting_network(RNG());
    gen_t odd_even(RNG(a), RNG(b));

public:
    void sort(size_t sz)
    {
        decltype(auto) G = sorting_network(sz, 1, 0, sz);
        while (!G.eof())
        {
            decltype(auto) p = G.next();
            sort_unit(p.first, p.second);
        }
    }
};
