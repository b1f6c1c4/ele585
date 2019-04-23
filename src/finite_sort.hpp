#include <utility>
#include <vector>
#include "fancy_choice.hpp"

#define FINITE_SORT_DEBUG
#define FINITE_SORT_DEBUG_EX
#ifdef FINITE_SORT_DEBUG_EX
#define SN_LOG(str) do { \
        sn_log(RNX()); \
        std::cerr << str << std::endl; \
    } while (false)
#define OE_LOG(str) do { \
        oe_log(RNX(a), RNX(b)); \
        std::cerr << str << std::endl; \
    } while (false)
#else
#define SN_LOG
#define OE_LOG
#endif

#define RNG(x) size_t N##x, size_t NS##x, size_t B##x, size_t NT##x
#define RNX(x) N##x, NS##x, B##x, NT##x

template <typename T, size_t NTotal>
class finite_sort
{
private:
#ifdef FINITE_SORT_DEBUG_EX
    inline void sn_log(RNG())
    {
        std::vector<int> objs{};
        objs.resize(std::max(NTotal, B + N * NS - NS + 1), 0);
        for (size_t i = 0; i < N; i++)
            objs[B + i * NS] |= 1;
        for (size_t i = 0; i < objs.size(); i++)
            if (objs[i])
                if (i < NTotal)
                    std::cerr << " | ";
                else
                    std::cerr << " $ ";
            else
                std::cerr << "   ";
    }
    inline void oe_log(RNG(a), RNG(b))
    {
        std::vector<int> objs{};
        objs.resize(std::max(NTotal, std::max(Ba + Na * NSa - NSa + 1, Bb + Nb * NSb - NSb + 1)), 0);
        for (size_t i = 0; i < Na; i++)
            objs[Ba + i * NSa] |= 1;
        for (size_t i = 0; i < Nb; i++)
            objs[Bb + i * NSb] |= 2;
        for (size_t i = 0; i < objs.size(); i++)
            if (objs[i] == 1)
                if (i < NTotal)
                    std::cerr << " a ";
                else
                    std::cerr << " A ";
            else if (objs[i] == 2)
                if (i < NTotal)
                    std::cerr << " b ";
                else
                    std::cerr << " B ";
            else if (objs[i] == 3)
                std::cerr << "!!!";
            else
                std::cerr << "   ";
    }
#endif

    static inline constexpr size_t ceil_2(size_t v) { return v - (v / 2); }
    static inline constexpr size_t floor_2(size_t v) { return v / 2; }

    template <size_t NTo>
    static inline constexpr size_t trim(size_t NT)
    {
        return std::min(NT, NTo);
    }

    inline void sort_unit(T &l, T& r);

    template <RNG()>
    inline void sorting_network() { sorting_network<RNX()>(fancy_choice<4>{}); }

#define SORTING_NETWORK_DECL(n, x) \
    template <RNG(), typename std::enable_if_t<(x)>* = nullptr> \
    inline void sorting_network(fancy_choice<n>)

#define SORTING_NETWORK_IMPL(n, x) \
    template <typename T, size_t NTotal> \
    template <RNG(), typename std::enable_if_t<(x)>*> \
    inline void finite_sort<T, NTotal>::sorting_network(fancy_choice<n>)

    template <RNG(a), RNG(b)>
    inline void odd_even() { odd_even<RNX(a), RNX(b)>(fancy_choice<4>{}); }

#define ODD_EVEN_DECL(n, x) \
    template <RNG(a), RNG(b), typename std::enable_if_t<(x)>* = nullptr> \
    inline void odd_even(fancy_choice<n>)

#define ODD_EVEN_IMPL(n, x) \
    template <typename T, size_t NTotal> \
    template <RNG(a), RNG(b), typename std::enable_if_t<(x)>*> \
    inline void finite_sort<T, NTotal>::odd_even(fancy_choice<n>)

    SORTING_NETWORK_DECL(2, N == 2);
    SORTING_NETWORK_DECL(2, N == 3);
    SORTING_NETWORK_DECL(2, N == 9);
    SORTING_NETWORK_DECL(2, N == 10);
    SORTING_NETWORK_DECL(2, N == 12);
    SORTING_NETWORK_DECL(2, N == 13);
    SORTING_NETWORK_DECL(2, N == 16);

    SORTING_NETWORK_DECL(1, N > 10 && N < 16)
    {
        SN_LOG("upgrade");
        sorting_network<N + 1, NS, B>();
    }

    SORTING_NETWORK_DECL(0, true);

    ODD_EVEN_DECL(4, Na == 0 || Nb == 0) { OE_LOG("trivial"); }
    ODD_EVEN_DECL(3, Na == 1 && Nb == 1)
    {
        OE_LOG("one-one");
        if (Ba < NTa && Bb < NTb)
            sort_unit(_d[Ba], _d[Bb]);
    }

    ODD_EVEN_DECL(0, true);

    T *_d;
#ifdef FINITE_SORT_DEBUG
    size_t _count;
#endif

public:
    inline void sort(T *d);
};

template <typename T, size_t NTotal>
inline void finite_sort<T, NTotal>::sort_unit(T &l, T& r)
{
#ifdef FINITE_SORT_DEBUG
    auto lv = &l - _d;
    auto rv = &r - _d;
    for (size_t i = 0; i < lv; i++)
        std::cerr << " | ";
    std::cerr << " X-";
    for (size_t i = lv + 1; i < rv; i++)
        std::cerr << "-+-";
    std::cerr << "-X ";
    for (size_t i = rv + 1; i < NTotal; i++)
        std::cerr << " | ";
    std::cerr << " [" << lv << "]--[" << rv << "]" << std::endl;
    _count++;
#endif
    if (r < l)
        std::swap(l, r);
}

#define X(l, r) sorting_network<2, ((r) - (l)) * NS, B + (l) * NS, NT>()

SORTING_NETWORK_IMPL(2, N == 2)
{
    SN_LOG("two");
    if (B + NS < NT)
        sort_unit(_d[B], _d[B + NS]);
}

SORTING_NETWORK_IMPL(2, N == 3)
{
    SN_LOG("three");
    X(0, 1), X(1, 2), X(0, 1);
}

SORTING_NETWORK_IMPL(2, N == 9)
{
    SN_LOG("nine");
    X(0, 1), X(1, 2), X(0, 1);
    X(3, 4), X(4, 5), X(3, 4);
    X(6, 7), X(7, 8), X(6, 7);
    X(0, 3), X(3, 6), X(0, 3);
    X(1, 4), X(4, 7), X(1, 4);
    X(2, 5), X(5, 8), X(2, 5);
    X(1, 3), X(5, 7);
    X(2, 6), X(4, 6), X(2, 4);
    X(2, 3), X(5, 6);
}

SORTING_NETWORK_IMPL(2, N == 10)
{
    SN_LOG("ten");
    X(0, 5), X(1, 6), X(2, 7), X(3, 8), X(4, 9);
    X(0, 3), X(1, 4), X(5, 8), X(6, 9);
    X(0, 2), X(3, 6), X(7, 9);
    X(0, 1), X(2, 4), X(5, 7), X(8, 9);
    X(1, 2), X(3, 5), X(4, 6), X(7, 8);
    X(1, 3), X(2, 5), X(4, 7), X(6, 8);
    X(2, 3), X(3, 4), X(6, 7), X(5, 6), X(4, 5);
}

SORTING_NETWORK_IMPL(2, N == 12)
{
    SN_LOG("twelve");
    sorting_network<4, NS, B + (0 * NS), NT>();
    sorting_network<4, NS, B + (4 * NS), NT>();
    sorting_network<4, NS, B + (8 * NS), NT>();
    X(1, 5), X(5, 9), X(1, 5), X(6, 10), X(2, 6), X(6, 10);
    X(0, 4), X(4, 8), X(0, 4), X(7, 11), X(3, 7), X(7, 11);
    X(1, 4), X(7, 10), X(3, 8);
    X(2, 3), X(2, 4), X(3, 5), X(8, 9), X(7, 9), X(6, 8);
    X(3, 4), X(5, 6), X(7, 8);
}

SORTING_NETWORK_IMPL(2, N == 13)
{
    SN_LOG("thirteen");
    X(0, 5), X(1, 7), X(3, 9), X(2, 4), X(6, 11), X(8, 12);
    X(0, 6), X(1, 3), X(2, 8), X(4, 12), X(5, 11), X(7, 9);
    X(0, 2), X(4, 5), X(6, 8), X(9, 10), X(11, 12);
    X(3, 12), X(5, 9), X(7, 8), X(10, 12);
    X(1, 5), X(2, 3), X(4, 7), X(8, 10), X(9, 11);
    X(0, 1), X(5, 6), X(8, 9), X(10, 11);
    X(1, 4), X(2, 5), X(3, 6), X(7, 8), X(9, 10);
    X(1, 2), X(3, 7), X(4, 5), X(6, 8);
    X(2, 4), X(3, 5), X(6, 7), X(8, 9);
    X(3, 4), X(5, 6);
}

SORTING_NETWORK_IMPL(2, N == 16)
{
    SN_LOG("sixteen");
#define STAGE(x) X(x + 0, x + 1), X(x + 2, x + 3), X(x + 0, x + 2), X(x + 1, x + 3)
    STAGE(0), STAGE(4), STAGE(8), STAGE(12);
#undef STAGE
#define STAGE(x) X(x + 0, x + 4), X(x + 1, x + 5), X(x + 2, x + 6), X(x + 3, x + 7)
    STAGE(0), STAGE(8);
#undef STAGE
    X(0, 8), X(1, 9), X(2, 10), X(3, 11), X(4, 12), X(5, 13), X(6, 14), X(7, 15);
    X(1, 2), X(3, 12), X(4, 8), X(5, 10), X(6, 9), X(7, 11), X(13, 14);
    X(1, 4), X(2, 8), X(7, 13), X(11, 14);
    X(2, 4), X(3, 8), X(5, 6), X(7, 12), X(9, 10), X(11, 13);
    X(3, 5), X(6, 8), X(7, 9), X(10, 12);
    X(3, 4), X(5, 6), X(7, 8), X(9, 10), X(11, 12);
    X(6, 7), X(8, 9);
}

SORTING_NETWORK_IMPL(0, true)
{
    SN_LOG("odd-even");

    constexpr auto P = ceil_2(N);
    constexpr auto Q = floor_2(N);

#define T(n, ns, b) (n), (ns), (b), trim<NT>((b) + (n) * (ns))
#define L T(P, NS, B + (0 * NS))
#define R T(Q, NS, B + (P * NS))
    sorting_network<L>();
    sorting_network<R>();
    odd_even<L, R>();
#undef L
#undef R
#undef T
}

ODD_EVEN_IMPL(0, true)
{
    OE_LOG("odd-even-impl");

    static_assert(NTa > Ba + Na * NSa - NSa, "NTa is not fully accessible");

#define Ta(n, ns, b) (n), (ns), (b), trim<NTa>((b) + (n) * (ns) + 1)
#define Tb(n, ns, b) (n), (ns), (b), trim<NTb>((b) + (n) * (ns) + 1)
    odd_even<
        Ta(ceil_2(Na),  2 * NSa, Ba + 0 * NSa),
        Tb(ceil_2(Nb),  2 * NSb, Bb + 0 * NSb)
        >();
    odd_even<
        Ta(floor_2(Na), 2 * NSa, Ba + 1 * NSa),
        Tb(floor_2(Nb), 2 * NSb, Bb + 1 * NSb)
        >();

    auto valid = false;
    size_t last = 0;
    for (size_t i = 1; i < Na; i++)
        if (!valid)
            valid = true, last = Ba + i * NSa;
        else
            valid = false, sort_unit(_d[last], _d[Ba + i * NSa]);
    for (size_t i = 0; i < Nb; i++)
        if (!valid)
            valid = true, last = Bb + i * NSb;
        else if (last < NTb && Bb + i * NSb < NTb)
            valid = false, sort_unit(_d[last], _d[Bb + i * NSb]);
}

#undef X

template <typename T, size_t NTotal>
inline void finite_sort<T, NTotal>::sort(T *d)
{
#ifdef FINITE_SORT_DEBUG
    _d = d;
#endif
    sorting_network<NTotal, 1, 0, NTotal>();
#ifdef FINITE_SORT_DEBUG
    std::cerr << "Total = " << _count << std::endl;
#endif
}
