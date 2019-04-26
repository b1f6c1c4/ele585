#pragma once

#include <iostream>
#include <cmath>
#include <utility>
#include <iterator>
#include <algorithm>
#include "sn_sort.hpp"

#ifndef X_USE_SN
#define X_USE_SN 16
#endif

#ifndef X_MAX_DEPTH_MULT
#define X_MAX_DEPTH_MULT 4
#endif

template <typename Iter>
using Tl = const typename std::iterator_traits<Iter>::reference;

template <typename Iter>
Tl<Iter> quick_random_pivots(Iter first, Iter last)
{
    auto q = (last - first) / 2;
    Iter v[3] = {
        first,
        first + q,
        last,
    };
#define X(l, r) if (*v[r] < *v[l]) std::iter_swap(v[l], v[r])
    X(0, 1); X(1, 2); X(0, 1);
#undef X
    return *v[1];
}

template <typename Iter>
std::pair<Iter, Iter> quick_partition(Iter first, Iter last, bool reversed)
{
    decltype(auto) p = quick_random_pivots(first, last);

    auto left = first;
    auto right = last;
    for (auto i = left; i <= right;)
    {
        if (reversed ? (p < *i) : (*i < p))
            std::iter_swap(left++, i++);
        else if (reversed ? (*i < p) : (p < *i))
            std::iter_swap(i, right--);
        else
            ++i;
    }
    return std::make_pair(left, right);
}

template <typename Iter>
void quick_sort(Iter begin, Iter end, bool reversed, size_t max_depth)
{
    while (true)
    {
        if (end - begin <= X_USE_SN)
        {
            sn_sort(begin, end, reversed);
            return;
        }

        if (!max_depth)
            break;

        decltype(auto) lr = quick_partition(begin, end - 1, reversed);

        auto dL = lr.first - begin;
        auto dR = end - lr.second;

        if (dL >= dR)
        {
            quick_sort(lr.second + 1, end, reversed, max_depth - 1);
            end = lr.first;
        }
        else
        {
            quick_sort(begin, lr.first, reversed, max_depth - 1);
            begin = lr.second + 1;
        }

        max_depth--;
    }

    std::make_heap(begin, end);
    std::sort_heap(begin, end);
}

template <typename Iter>
void quick_sort(Iter begin, Iter end, bool reversed = false)
{
    quick_sort(begin, end, reversed, std::log2(end - begin) * X_MAX_DEPTH_MULT);
}
