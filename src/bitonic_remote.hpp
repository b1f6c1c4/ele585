#pragma once

#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstdint>
#include "quick_sort.hpp"

#define IS_POW_2(x) ((x) && !((x) & ((x) - 1)))

#ifndef X_MPI_MSG
// Reduce this to 2KiB can avoid deadlock
#define X_MPI_MSG static_cast<size_t>(32ull * 1024ull) // 32KiB
#endif

#define ASC true
#define DESC false

template <typename T>
class bitonic_remote
{
public:
    typedef int tag_t;

    virtual ~bitonic_remote()
    {
        delete _d;
        delete _recv;
    }

    bitonic_remote(const bitonic_remote &) = delete;
    bitonic_remote(bitonic_remote &&other) noexcept
        : My(other.My), NMach(other.NMach), NMem(other.NMem), NSec(other.NSec), _d(other._d), _recv(other._recv)
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
        NSec = other.NSec;

        delete _d;
        delete _recv;

        _d = other._d;
        _recv = other._recv;

        other._d = nullptr;
        other._recv = nullptr;

        return *this;
    }

    void execute(size_t my)
    {
        My = my;
        bitonic_sort_init((My % 2) == 0 ? ASC : DESC);
        bitonic_sort_merge(ASC, 0);
    }

protected:

    virtual void send_mem(const T *d, size_t sz, size_t partner, tag_t tag) = 0;
    virtual void recv_mem(T *d, size_t sz, size_t partner, tag_t tag) = 0;

    virtual void load_sec(size_t sec, size_t offset, T *d, size_t sz) = 0;
    virtual void write_sec(size_t sec, size_t offset, const T *d, size_t sz) = 0;

    bitonic_remote(size_t nmach, size_t nmem, size_t nsec)
        : My(nmach), NMach(nmach), NMem(nmem), NSec(nsec), _d(new T[nmem]), _recv(new T[NMsg])
    {
        if (!IS_POW_2(nmach))
            throw std::runtime_error("NMach is not pow of 2");
        if (!IS_POW_2(nmem))
            throw std::runtime_error("NMem is not pow of 2");
        if (!IS_POW_2(nsec))
            throw std::runtime_error("NSec is not pow of 2");
    }

    typedef bool dir_t;
    static constexpr auto NMsg = X_MPI_MSG / sizeof(T);
    size_t My;
    size_t NMach;
    size_t NMem;
    size_t NSec;

private:

    T *_d;
    T *_recv;

    void initial_sort_mem(dir_t dir)
    {
        quick_sort(_d, _d + NMem, dir == DESC);
    }

    // Requires:
    //     MEM[d, d + sz / 2)      is  x-ordered
    //     MEM[d + sz / 2, d + sz) is !x-ordered
    // Ensures:
    //     MEM[d, d + sz)          is  dir-ordered
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
            for (size_t i = 0; i < half; i++)
                if (dir == ASC ? (d[i + half] < d[i]) : (d[i] < d[i + half]))
                    std::swap(d[i], d[i + half]);

            bitonic_sort_mem(d + half, half, dir);
            sz = half;
        }
    }

    // Requires:
    //     MEM[0, 0.5) is  x-ordered
    //     MEM[0.5, 1) is !x-ordered
    // Ensures:
    //     MEM[0, 1)   is  dir-ordered
    void bitonic_sort_mem(dir_t dir)
    {
        bitonic_sort_mem(_d, NMem, dir);
    }

    // Requires:
    //     F[my][bsec, bsec + nsec / 2)        is  x-ordered
    //     F[my][bsec + nsec / 2, bsec + nsec) is !x-ordered
    // Ensures:
    //     F[my][bsec, bsec + nsec)            is  dir-ordered
    void bitonic_sort_secs(size_t bsec, size_t nsec, dir_t dir)
    {
        const auto half = NMem / 2;
        while (nsec)
        {
            if (nsec == 1)
            {
                load_sec(bsec, 0, _d, NMem);
                bitonic_sort_mem(dir);
                write_sec(bsec, 0, _d, NMem);
                return;
            }

            for (size_t i = 0; i < nsec / 2; i++)
            {
                const auto lsec = bsec + i;
                const auto rsec = bsec + i + nsec / 2;
                const auto dl = _d;
                const auto dr = _d + half;

                load_sec(lsec, 0, dl, half);
                load_sec(rsec, 0, dr, half);
                bitonic_sort_mem(dir);
                write_sec(lsec, 0, dl, half);
                write_sec(rsec, 0, dr, half);

                load_sec(lsec, half, dl, half);
                load_sec(rsec, half, dr, half);
                bitonic_sort_mem(dir);
                write_sec(lsec, half, dl, half);
                write_sec(rsec, half, dr, half);
            }

            bitonic_sort_secs(bsec + nsec / 2, nsec / 2, dir);
            nsec /= 2;
        }
    }

    // If kind == ASC:
    // Requires:
    //     F[my]     [0, NSec) is  x-ordered
    //     F[partner][0, NSec) is !x-ordered
    // Ensures:
    //     F[my]     [0, NSec) is ^dir - ordered
    //                    <=dir=>
    //     F[partner][0, NSec) is vdir - ordered
    //
    // If kind == DESC:
    // Requires:
    //     F[partner][0, NSec) is  x-ordered
    //     F[my]     [0, NSec) is !x-ordered
    // Ensures:
    //     F[partner][0, NSec) is ^dir - ordered
    //                    <=dir=>
    //     F[my]     [0, NSec) is vdir - ordered
    void bitonic_cross_pair(dir_t kind, size_t partner, dir_t dir, tag_t tag)
    {
        const auto base_base_tag = tag << static_cast<int>(1 + std::log2(NSec));
        for (size_t sec = 0; sec < NSec; sec++)
        {
            const auto base_tag = base_base_tag << static_cast<int>(1 + std::log2(NMem / NMsg)) | sec;
            load_sec(sec, 0, _d, NMem);
            for (auto ptr = _d; ptr < _d + NMem; ptr += NMsg)
            {
                const auto mx = std::min(NMsg, static_cast<size_t>(_d + NMem - ptr));
                const auto tg = base_tag | (ptr - _d) / NMsg;
                send_mem(ptr, mx, partner, tg);
                recv_mem(_recv, mx, partner, tg);
                for (size_t i = 0; i < mx; i++)
                    if (((dir == ASC) != (kind == ASC))
                        ? (_recv[i] < ptr[i])
                        : (ptr[i] < _recv[i]))
                        ptr[i] = _recv[i];
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
    //     F[my][0, NSec) is  dir-ordered
    void bitonic_sort_init(dir_t dir)
    {
        for (size_t p = 0; p < std::log2(NSec); p++)
        {
            const auto nsec = static_cast<size_t>(1) << p;
            for (size_t i = 0; i < NSec; i += nsec)
                bitonic_sort_secs(i, nsec, ((i / nsec) % 2 == 0) ? ASC : DESC);
        }
        bitonic_sort_secs(0, NSec, dir);
    }

    // Requires:
    //     F[@all][0, NSec) is @dir-ordered each
    // Ensures:
    //     F[@all][0, NSec) is @dir-ordered all
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
