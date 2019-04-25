#pragma once

#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstdint>
#include "quick_sort.hpp"

#define IS_POW_2(x) (!(x) && !((x) & ((x) - 1)))

#ifndef X_MPI_MSG
#define X_MPI_MSG static_cast<size_t>(32ull * 1024ull * 1024ull) // 32MiB
#endif

#define ASC true
#define DESC false

template <typename T>
class bitonic_remote
{
protected:
    typedef int tag_t;

    virtual void send_mem(const T *d, size_t sz, size_t partner, tag_t tag) = 0;
    virtual void recv_mem(T *d, size_t sz, size_t partner, tag_t tag) = 0;

private:
    typedef bool dir_t;
    static constexpr auto NMsg = X_MPI_MSG / sizeof(T);
    const size_t My;
    const size_t NMach;
    const size_t NMem;
    const size_t NSec;
    std::fstream _f;

    T *_d;
    T *_recv;

    void load_sec(size_t sec, size_t offset, T *d, size_t sz)
    {
        _f.seekg(NMem * sec + offset, std::ios::beg);
        _f.read(reinterpret_cast<char *>(d), sizeof(T) * sz);
    }

    void write_sec(size_t sec, size_t offset, const T *d, size_t sz)
    {
        _f.seekg(NMem * sec + offset, std::ios::beg);
        _f.write(reinterpret_cast<char *>(d), sizeof(T) * sz);
    }

    void initial_sort_mem(dir_t dir)
    {
        // TODO: respect dir
        quick_sort(_d, _d + NMem);
    }

    void bitonic_sort_mem(T *d, size_t sz, dir_t dir)
    {
        while (sz >= 2)
        {
            if (sz == 2)
            {
                if (dir == ASC ? (d[1] < d[0]) : (d[0] < d[1]))
                    std::swap(d[0], d[1]);
                return;
            }

            const size_t half = sz / 2;
            for (auto i = 0; i < half; i++)
                if (dir == ASC ? (d[i + half] < d[i]) : (d[i] < d[i + half]))
                    std::swap(d[i], d[i + half]);

            bitonic_sort_mem(d + half, half);
            sz = half;
        }
    }

    // Requires:
    //     MEM[0, 0.5) is  dir-ordered
    //     MEM[0.5, 1) is !dir-ordered
    // Ensures:
    //     MEM[0, 1)   is  dir-ordered
    void bitonic_sort_mem(dir_t dir)
    {
        bitonic_sort_mem(_d, NMem, dir);
    }

    // Requires:
    //     F[my][bsec, bsec + nsec / 2)        is  dir-ordered
    //     F[my][bsec + nsec / 2, bsec + nsec) is !dir-ordered
    // Ensures:
    //     F[my][bsec, bsec + nsec)            is  dir-ordered
    void bitonic_sort_secs(size_t bsec, size_t nsec, dir_t dir)
    {
        const auto half = NMem / 2;
        while (nsec > 1)
        {
            for (size_t i = 0; i < nsec / 2; i++)
            {
                const auto lsec = bsec + i;
                const auto rsec = bsec + i + nsec / 2;

                load_sec(lsec, 0, _d, half);
                load_sec(rsec, 0, _d + half, half);
                bitonic_sort_mem(dir);
                write_sec(lsec, 0, _d, half);
                write_sec(rsec, 0, _d + half, half);

                load_sec(lsec, half, _d, half);
                load_sec(rsec, half, _d + half, half);
                bitonic_sort_mem(dir);
                write_sec(lsec, half, _d, half);
                write_sec(rsec, half, _d + half, half);
            }

            bitonic_sort_secs(bsec + nsec / 2, nsec / 2, dir);
            nsec /= 2;
        }
    }

    // If kind == ASC:
    // Requires:
    //     F[my]     [0, NSec) is  dir-ordered
    //     F[partner][0, NSec) is !dir-ordered
    // Ensures:
    //     F[my][0, NSec) <=dir=> F[partner][bsec, bsec + nsec)
    //
    // If kind == DESC:
    // Requires:
    //     F[partner][0, NSec) is  dir-ordered
    //     F[my]     [0, NSec) is !dir-ordered
    // Ensures:
    //     F[partner][0, NSec) is ^dir - ordered
    //                    <=dir=>
    //     F[my]     [0, NSec) is vdir - ordered
    void bitonic_cross_pair(dir_t kind, size_t partner, dir_t dir, tag_t tag)
    {
        const auto base_base_tag = tag << static_cast<int>(1 + std::log2(NMsg));
        for (size_t sec = 0; sec < NSec; sec++)
        {
            const auto base_tag = base_base_tag << static_cast<int>(std::log2(NSec)) | sec;
            load_sec(sec, 0, _d, NMem);
            for (auto ptr = _d; ptr < _d + NMem; ptr += NMsg)
            {
                const auto tg = base_tag | (ptr - _d) / NMem;
                send_mem(ptr, NMsg, partner, tg);
                recv_mem(_recv, NMsg, partner, tg);
                for (size_t i = 0; i < NMem - (ptr - _d); i++)
                    if ((dir == ASC) != (kind == ASC) ? (*ptr < _recv[i]) : (_recv[i] < *ptr))
                        *ptr = _recv[i];
            }
            write_sec(sec, 0, _d, NMem);
        }
    }

    // Requires:
    //     F[@level][0, NSec) is @dir-ordered each
    // Ensures:
    //     F[@level][0, NSec) is @dir-ordered all
    void bitonic_sort_prefix(size_t level, dir_t dir, tag_t tag)
    {
        const auto base_tag = tag << static_cast<int>(1 + std::log2(std::log2(NMach)));
        auto mask = static_cast<size_t>(1) << level;
        while (mask)
        {
            bitonic_cross_pair((My & mask) ? ASC : DESC, My ^ mask, dir, base_tag | level);
            level--, mask >>= 1;
        }
        bitonic_sort_secs(0, NSec, dir);
    }

    // Requires:
    // Ensures:
    //     F[@all][0, NSec) is @dir-ordered all
    void bitonic_sort_all(dir_t dir, tag_t tag)
    {
        const auto base_tag = tag << static_cast<int>(1 + std::log2(std::log2(NMach)));
        bitonic_sort_secs(0, NSec, dir);
        for (size_t p = 0; p < std::log2(NMach); p++)
            bitonic_sort_prefix(p, dir, base_tag | p);
    }

public:
    bitonic_remote(size_t nmach, size_t nmem, size_t nsec, const std::string &fn)
        : My(nmach), NMach(nmach), NMem(nmem), NSec(nsec), _f(fn, std::ios::binary), _d(new T[nmem]), _recv(new T[NMsg])
    {
        if (!IS_POW_2(nmach) || !IS_POW_2(nmem) || !IS_POW_2(nsec))
            throw;
    }

    virtual ~bitonic_remote()
    {
        delete _d;
        delete _recv;
    }

    void execute(size_t my)
    {
        My = my;
        bitonic_sort_all(ASC, 0);
    }
};
