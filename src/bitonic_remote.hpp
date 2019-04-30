#pragma once

#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <limits>
#include <iomanip>
#include "timed.hpp"
#include "logger.hpp"
#include "quick_sort.hpp"

#define BITONIC_MPI_INFO
// #define BITONIC_MPI_CROSS_DEBUG
// #define BITONIC_MPI_CROSS_TRACE

#define IS_POW_2(x) ((x) && !((x) & ((x) - 1)))

#ifdef BITONIC_MPI_INFO
#define LOG(...) write_log(NMach, My, __VA_ARGS__)
#endif

#define ASC true
#define DESC false

template <typename T, size_t NDiv, size_t NMin>
class bitonic_remote
{
public:
    typedef int tag_t;

    aggregated _comp;
    aggregated _comm;

    virtual ~bitonic_remote()
    {
        delete [] _recv;
        _recv = nullptr;
    }

    bitonic_remote(const bitonic_remote &) = delete;
    bitonic_remote(bitonic_remote &&other) noexcept
        : My(other.My), NMach(other.NMach),
          NMem(other.NMem), NMsg(other.NMsg),
          _d(other._d), _recv(other._recv)
    {
        other._d = nullptr;
        other._recv = nullptr;
    }

    bitonic_remote &operator=(const bitonic_remote &) = delete;
    bitonic_remote &operator=(bitonic_remote &&other) noexcept
    {
        if (this == *other)
            return *this;

        My = other.My;
        NMach = other.NMach;
        NMem = other.NMem;
        NMsg = other.NMsg;

        delete [] _recv;

        _d = other._d;
        _recv = other._recv;

        other._d = nullptr;
        other._recv = nullptr;

        return *this;
    }

#define TIMED(x, ...) ({ decltype(auto) g = x.fork(); __VA_ARGS__; })

    void execute(size_t my)
    {
        My = my;
        bitonic_sort_init((My % 2) == 0 ? ASC : DESC);
        bitonic_sort_merge(ASC, 0);
    }

protected:

    virtual void exchange_mem(
        size_t sz, size_t partner, tag_t tag,
        const T *source, T *dest) = 0;

    bitonic_remote(size_t nmach, size_t nmem, size_t nmsg, T *d)
        : My(nmach), NMach(nmach), NMem(nmem), NMsg(nmsg),
          _d(d), _recv(new T[nmsg])
    {
        if (nmem < 2)
            throw std::runtime_error("NMem is too small");
        if (!IS_POW_2(nmach))
            throw std::runtime_error("NMach is not pow of 2");
        if (!IS_POW_2(nmem))
            throw std::runtime_error("NMem is not pow of 2");
    }

    typedef bool dir_t;
    size_t My;
    size_t NMach;
    size_t NMem;
    size_t NMsg;

    T *_d;

private:

    T *_recv;
    T _fptx[NDiv];
    T _fprx[NDiv];

    void initial_sort_mem(dir_t dir)
    {
        decltype(auto) g = _comp.fork();
        /*
        std::sort(_d, _d + NMem);
        if (dir == DESC)
            std::reverse(_d, _d + NMem);
        return;
        */
        quick_sort(_d, _d + NMem, dir == DESC);
    }

    // Requires:
    //     MEM[d, d + sz)          is  v-ordered or ^-ordered
    // Ensures:
    //     MEM[d, d + sz)          is  dir-ordered
    void bitonic_sort_mem(T *d, size_t sz, dir_t dir)
    {
#define Q(l, r) (dir == ASC ? (d[r] < d[l]) : (d[l] < d[r]))
#define X(l, r) std::swap(d[l], d[r])

        while (sz >= 2)
        {
            if (sz == 2)
            {
                if (Q(0, 1))
                    X(0, 1);
                return;
            }

            const auto half = sz / 2;
            const auto dl = Q(0, half);
            const auto dr = Q(half - 1, sz - 1);

            if (dl && dr)
            {
                for (size_t i = 0; i < half; i++)
                    X(i, i + half);
            }
            else if (dl != dr)
            {
                size_t left = 0, right = half;
                while (left < right)
                {
                    size_t v = (left + right) / 2;
                    if (dr == Q(v, v + half))
                        right = v;
                    else
                        left = v + 1;
                }

                if (dr)
                {
                    for (auto i = right; i < half; i++)
                        X(i, i + half);
                }
                else
                {
                    for (size_t i = 0; i < right; i++)
                        X(i, i + half);
                }
            }

            bitonic_sort_mem(d + half, half, dir);
            sz = half;
        }
#undef Q
#undef X
    }

    // Requires:
    //     MEM[0, NMem) is  v-ordered or ^-ordered
    // Ensures:
    //     MEM[0, NMem) is  dir-ordered
    void bitonic_sort_mem(dir_t dir)
    {
        decltype(auto) g = _comp.fork();
        bitonic_sort_mem(_d, NMem, dir);
    }

    // If kind == ASC:
    // Requires:
    //     F[my]     [0, NMem) ... F[partner][0, NMem) is v-ordered or ^-ordered
    // Ensures:
    //     F[my]     [0, R)    <=dir=> F[partner][0, R)
    //     F[my]     [R, NMem) <=dir=> F[partner][R, NMem)
    std::pair<size_t, size_t> intersection_cross_pair(dir_t kind, size_t partner, dir_t dir, tag_t tag)
    {
#define Q0(p, q) (((dir == ASC) != (kind == ASC)) ? (q < p) : (p < q))
#define Q(v) Q0(_fptx[v], _fprx[v])

        size_t left = 0, right = NMem;
        auto extl = true, extr = true;
        const auto shift = static_cast<int>(1 + std::log(NMem) / std::log(NDiv));
        const auto base_tag = tag << shift;
        for (size_t iter = 0; right - left > NMin && right - left > NDiv; iter++)
        {
            auto delta = (right - left) / NDiv;
            if (!delta)
                delta = 1;
            size_t ndiv;
            for (ndiv = 0; ndiv < NDiv - 1; ndiv++)
            {
                if (left + delta * ndiv >= right)
                    break;
                _fptx[ndiv] = _d[left + delta * ndiv];
            }
            _fptx[ndiv] = _d[right - 1];
            ndiv++;
            TIMED(_comm, exchange_mem(ndiv, partner, base_tag | (shift - 1 - iter), _fptx, _fprx));

            const auto dl = Q(0);
            const auto dr = Q(ndiv - 1);

            if (dl == dr)
            {
                if (extl == extr)
                {
                    extl = dl, extr = dr;
                    left = 0, right = 0;
                }
                else if (dl == extr)
                    right = left;
                else if (dr == extl)
                    left = right;
                else
                    throw std::runtime_error("Bitonicity broken");
                break;
            }

            extl = dl, extr = dr;

            size_t sleft = 0, sright = ndiv - 1;
            while (sleft < sright)
            {
                const auto v = (sleft + sright) / 2;
                if (Q(v) == dr)
                    sright = v;
                else // if (Q(v) == dl)
                    sleft = v + 1;
            }
            auto old = left;
            if (sright != 0)
                left = old + delta * (sright - 1) + 1;
            if (sright != ndiv - 1)
                right = old + delta * sright;
        }

        return std::make_pair(extl ? 0 : left, extr ? NMem : right);
    }

    // If kind == ASC:
    // Requires:
    //     F[my]      ... F[partner] is v-ordered or ^-ordered
    // Ensures:
    //     F[my]      is ^dir - ordered
    //                    <=dir=>
    //     F[partner] is vdir - ordered
    //
    // If kind == DESC:
    // Requires:
    //     F[partner] ... F[my]      is v-ordered or ^-ordered
    // Ensures:
    //     F[partner] is ^dir - ordered
    //                    <=dir=>
    //     F[my]      is vdir - ordered
    void bitonic_cross_pair(dir_t kind, size_t partner, dir_t dir, tag_t tag)
    {
        const auto base_tag = tag << static_cast<int>(1 + std::log2(NMem / NMsg));
#ifdef BITONIC_MPI_CROSS_TRACE
        for (size_t i = 0; i < NMem; i++)
            std::cerr << " " << _d[i];
        std::cerr << std::endl;
#endif
#ifdef BITONIC_MPI_CROSS_DEBUG
        size_t begin = NMem, end = 0;
        for (size_t i = 0; i < NMem; i += NMsg)
        {
            const auto mx = std::min(NMsg, NMem - i);
            const auto tg = base_tag | i / NMsg;
            TIMED(_comm, exchange_mem(mx, partner, tg, _d + i, _recv));
            {
                decltype(auto) g = _comp.fork();
                for (size_t j = 0; j < mx; j++)
                {
                    if (Q0(_d[i + j], _recv[j]))
                    {
                        begin = std::min(begin, i + j);
                        end = std::max(end, i + j + 1);
                    }
                }
            }
        }
#endif

        decltype(auto) res = intersection_cross_pair(kind, partner, dir, tag);
        const auto left = res.first;
        const auto right = res.second;
#ifdef BITONIC_MPI_CROSS_DEBUG
        if (begin < left || end > right)
        {
            LOG("we die ", left, " < ", begin, " < ", end, " < ", right);
            std::ofstream fout(std::string("dump-") + std::to_string(My), std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
            fout.write(reinterpret_cast<const char *>(_d), sizeof(T) * NMem);
            std::exit(233);
        }
#endif

        for (auto i = left; i < right; i += NMsg)
        {
            const auto mx = std::min(NMsg, right - i);
            const auto tg = base_tag | (i - left) / NMsg;
            TIMED(_comm, exchange_mem(mx, partner, tg, _d + i, _recv));
            {
                decltype(auto) g = _comp.fork();
                for (size_t j = 0; j < mx; j++)
                    if (Q0(_d[i + j], _recv[j]))
                        _d[i + j] = _recv[j];
            }
        }
#ifdef BITONIC_MPI_CROSS_TRACE
        for (size_t i = 0; i < NMem; i++)
            std::cerr << " " << _d[i];
        std::cerr << std::endl;
#endif
    }
#undef Q0
#undef Q

    // Requires:
    //     F[@level] is @dir-ordered each
    // Ensures:
    //     F[@level] is @dir-ordered all
    void bitonic_sort_prefix(size_t level, dir_t dir, tag_t tag)
    {
        const auto coarse = level;
        const auto base_tag = tag << static_cast<int>(1 + std::log2(std::log2(NMach)));
        auto mask = static_cast<size_t>(1) << level;
        while (mask)
        {
            LOG("Level b.", coarse, ".", coarse - level);
            bitonic_cross_pair((My & mask) ? ASC : DESC, My ^ mask, dir, base_tag | level);
            level--, mask >>= 1;
        }
        LOG("Level b.", coarse, ".", coarse - level);
        bitonic_sort_mem(dir);
    }

    // Requires:
    // Ensures:
    //     F[my] is  dir-ordered
    void bitonic_sort_init(dir_t dir)
    {
        LOG("Level a");
        initial_sort_mem(dir);
    }

    // Requires:
    //     F[@all] is @dir-ordered each
    // Ensures:
    //     F[@all] is @dir-ordered all
    void bitonic_sort_merge(dir_t dir, tag_t tag)
    {
        const auto base_tag = tag << static_cast<int>(1 + std::log2(std::log2(NMach)));
        for (size_t p = 0; p < std::log2(NMach); p++)
        {
            const auto dirx = ((My >> (p + 1)) % 2 == 0) ? ASC : DESC;
            const auto diry = (dirx != dir) ? DESC : ASC;
            bitonic_sort_prefix(p, diry, base_tag | p);
        }
    }
};

#undef LOG
#undef ASC
#undef DESC
