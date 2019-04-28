#pragma once

#include <iostream>
#include <cmath>
#include <utility>
#include <iterator>
#include <algorithm>
#include "sn_sort.hpp"
#include "check_ordering.hpp"

// #define QUICK_SORT_DEBUG
// #define QUICK_SORT_TRACE

#ifndef X_USE_SN
#define X_USE_SN 16
#endif

#ifndef X_MAX_DEPTH_MULT
#define X_MAX_DEPTH_MULT 4
#endif

#define LOG(msg, begin, end) \
    do { \
        std::cerr << msg; \
        for (auto it = begin; it != end; ++it) \
            std::cerr << " " << *it / 1e17; \
        std::cerr << std::endl; \
    } while (false)

#ifdef QUICK_SORT_TRACE
#define TLOG(...) LOG(__VA_ARGS__)
#else
#define TLOG(...)
#endif

template <typename Iter>
using T = const typename std::iterator_traits<Iter>::value_type;

template <typename Iter>
T<Iter> quick_random_pivots(Iter first, Iter last)
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
#ifdef QUICK_SORT_TRACE
        for (auto it = first; it <= last; ++it)
        {
            if (it == i)
                std::cerr << "!";
            if (it == left && it == right)
                std::cerr << " <" << *it << ">";
            else if (it == left)
                std::cerr << " <" << *it;
            else if (it == right)
                std::cerr << " " << *it << ">";
            else
                std::cerr << " " << *it;
        }
        if (p < *i)
            std::cerr << "GT";
        else if (*i < p)
            std::cerr << "LT";
        else
            std::cerr << "EQ";
        std::cerr << std::endl;
#endif
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
    for (; ; max_depth--)
    {
        TLOG(begin, end);
        if (end - begin <= X_USE_SN)
        {
            sn_sort(begin, end, reversed);
#ifdef QUICK_SORT_DEBUG
            if (!check_ordering(begin, end - begin, reversed))
            {
                LOG("sn_sort", begin, end);
                std::exit(66);
            }
#endif
            TLOG(begin, end);
            return;
        }

        if (!max_depth)
        {
            std::make_heap(begin, end);
            std::sort_heap(begin, end);
#ifdef QUICK_SORT_DEBUG
            if (!check_ordering(begin, end - begin, reversed))
            {
                LOG("sort_heap", begin, end);
                std::exit(66);
            }
#endif
            return;
        }

        decltype(auto) lr = quick_partition(begin, end - 1, reversed);
#ifdef QUICK_SORT_TRACE
        for (auto it = begin; it != end; ++it)
            if (it == lr.first && it == lr.second)
                std::cerr << " <" << *it << ">";
            else if (it == lr.first)
                std::cerr << " <" << *it;
            else if (it == lr.second)
                std::cerr << " " << *it << ">";
            else
                std::cerr << " " << *it;
        std::cerr << std::endl;
#endif

        auto dL = lr.first - begin;
        auto dR = end - lr.second - 1;

#ifdef QUICK_SORT_TRACE
        std::cerr << "dL=" << dL << " dR=" << dR << std::endl;
#endif

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
    }
}

template <typename Iter>
void quick_sort(Iter begin, Iter end, bool reversed = false)
{
    quick_sort(begin, end, reversed, std::log2(end - begin) * X_MAX_DEPTH_MULT);
}

#undef LOG
