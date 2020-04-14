#pragma once

#include <iostream>
#include <cmath>
#include <utility>
#include <iterator>
#include <algorithm>
#include "sn_sort.hpp"

#ifndef X_USE_SN
#define X_USE_SN 32
#endif

#ifndef X_MAX_DEPTH_MULT
#define X_MAX_DEPTH_MULT 3
#endif

template <typename Iter>
void dp_random_pivots(Iter first, Iter last)
{
    auto q = (last - first) / 3;
    Iter v[4] = {
        first,
        first + q,
        last - q,
        last,
    };
#define X(l, r) if (*v[r] < *v[l]) std::iter_swap(v[l], v[r])
    X(0, 1); X(2, 3); X(0, 2); X(1, 3); X(1, 2);
#undef X
    std::iter_swap(first, v[1]);
    std::iter_swap(last - 1, v[2]);
}

template <typename Iter>
std::pair<Iter, Iter> dp_partition(Iter first, Iter last)
{
    if (*last < *first)
        std::iter_swap(first, last);

    typedef const typename std::iterator_traits<Iter>::reference Tl;

    Tl p = *first;
    Tl q = *last;

    auto left = first + 1;
    auto right = last - 1;
    for (auto i = left; i < right; ++i)
    {
        if (*i < p)
        {
            std::iter_swap(i, left++);
            continue;
        }
        if (*i < q)
            continue;

        while (q < *right && i < right)
            --right;
        std::iter_swap(i, right--);
        if (*i < p)
            std::iter_swap(i, left++);
    }
    std::iter_swap(first, --left);
    std::iter_swap(last, ++right);
    return std::make_pair(left, right);
}

template <typename Iter>
void dp_sort(Iter begin, Iter end, size_t max_depth)
{
    while (true)
    {
        if (end - begin <= X_USE_SN)
        {
            sn_sort(begin, end);
            return;
        }

        if (!max_depth)
            break;

        dp_random_pivots(begin, end - 1);
        decltype(auto) lr = dp_partition(begin, end - 1);

        auto dL = lr.first - begin;
        auto dM = lr.second - lr.first;
        auto dR = end - lr.second;

        if (dL >= dM && dL >= dR)
        {
            dp_sort(lr.first + 1, lr.second, max_depth - 1);
            dp_sort(lr.second, end, max_depth - 1);
            end = lr.first;
        }
        else if (dR >= dM && dR >= dL)
        {
            dp_sort(begin, lr.first, max_depth - 1);
            dp_sort(lr.first + 1, lr.second, max_depth - 1);
            begin = lr.second + 1;
        }
        else
        {
            dp_sort(begin, lr.first, max_depth - 1);
            dp_sort(lr.second, end, max_depth - 1);
            begin = lr.first, end = lr.second;
        }

        max_depth--;
    }

    std::make_heap(begin, end);
    std::sort_heap(begin, end);
}

template <typename Iter>
void dp_sort(Iter begin, Iter end)
{
    dp_sort(begin, end, std::log2(end - begin) * X_MAX_DEPTH_MULT);
}
