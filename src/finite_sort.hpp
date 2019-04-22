#include <utility>
#include <limits>
#include "fancy_choice.hpp"

#define FINITE_SORT_DEBUG
#define FINITE_SORT_DEBUG_EX
#ifdef FINITE_SORT_DEBUG_EX
#define LOG_FUNC(str) do { \
        std::cerr << "sort_scatter<" << N << ", " << NS << ", "; \
        if (B != INVALID) \
            std::cerr << B; \
        std::cerr << ">(d[" << d - _base << "]) " str << std::endl; \
    } while (false)
#else
#define LOG_FUNC
#endif

#define INVALID PTRDIFF_MAX

template <typename T, size_t NTotal>
class finite_sort
{
private:
    template <ptrdiff_t B>
    static inline constexpr ptrdiff_t bias(size_t dist)
    {
        if (B == INVALID)
            return INVALID;
        return B - dist;
    }

    inline void sort_unit(T &l, T& r);

    template <size_t N, size_t NS, ptrdiff_t B = INVALID>
    inline void sort_scatter(T *d)
    {
        sort_scatter<N, NS, B>(d, fancy_choice<3>{});
    }

    template <size_t N, size_t NS, ptrdiff_t B = INVALID,
             typename std::enable_if_t<
                 (N < 2 || B != INVALID && (B <= 0 || N * NS - NS + 1 <= B))
                 >* = nullptr>
    inline void sort_scatter(T *d, fancy_choice<3>)
    {
        LOG_FUNC("trivial");
    }

#define SORT_SCATTER_FOR_N_DECL(x) \
    template <size_t N, size_t NS, ptrdiff_t B = INVALID, \
             typename std::enable_if_t<(x)>* = nullptr> \
    inline void sort_scatter(T *d, fancy_choice<2>)

#define SORT_SCATTER_FOR_N_IMPL(x) \
    template <typename T, size_t NTotal> \
    template <size_t N, size_t NS, ptrdiff_t B, \
             typename std::enable_if_t<(x)>*> \
    inline void finite_sort<T, NTotal>::sort_scatter(T *d, fancy_choice<2>)

    SORT_SCATTER_FOR_N_DECL(N == 2);
    SORT_SCATTER_FOR_N_DECL(N == 3);
    SORT_SCATTER_FOR_N_DECL(N == 9);
    SORT_SCATTER_FOR_N_DECL(N == 10);
    SORT_SCATTER_FOR_N_DECL(N == 12);
    SORT_SCATTER_FOR_N_DECL(N == 13);
    SORT_SCATTER_FOR_N_DECL(N == 16);

    template <size_t N, size_t NS, ptrdiff_t B = INVALID, \
             typename std::enable_if_t<(N > 10 && N < 20)>* = nullptr> \
    inline void sort_scatter(T *d, fancy_choice<1>)
    {
        sort_scatter<N + 1, NS, B>(d);
    }

    template <size_t N, size_t NS, ptrdiff_t B = INVALID>
    inline void sort_scatter(T *d, fancy_choice<0>);

    template <size_t N, size_t NS, ptrdiff_t B>
    friend class sorter;

#ifdef FINITE_SORT_DEBUG
    const T *_base;
    size_t _count;
#endif

public:
    inline void sort(T *d);
};

template <typename T, size_t NTotal>
inline void finite_sort<T, NTotal>::sort_unit(T &l, T& r)
{
#ifdef FINITE_SORT_DEBUG
    auto lv = &l - _base;
    auto rv = &r - _base;
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

#define F(l, r) sort_scatter<2, ((r) - (l)) * NS, bias<B>((l) * NS)>(d + (l) * NS)
#define X(l, r) sort_unit(d[(l) * NS], d[(r) * NS])

SORT_SCATTER_FOR_N_IMPL(N == 2)
{
    LOG_FUNC("two");
    sort_unit(d[0], d[NS]);
}

SORT_SCATTER_FOR_N_IMPL(N == 3)
{
    LOG_FUNC("three");
    if (B > NS)
        F(0, 1), X(1, 2), X(0, 1);
    else
        F(1, 2), X(0, 1), X(1, 2);
}

SORT_SCATTER_FOR_N_IMPL(N == 9)
{
    LOG_FUNC("nine");
    F(0, 1), X(1, 2), X(0, 1);
    F(3, 4), X(4, 5), X(3, 4);
    F(6, 7), X(7, 8), X(6, 7);
    X(0, 3), X(3, 6), X(0, 3);
    X(1, 4), X(4, 7), X(1, 4);
    X(2, 5), X(5, 8), X(2, 5);
    X(1, 3), X(5, 7);
    X(2, 6), X(4, 6), X(2, 4);
    X(2, 3), X(5, 6);
}

SORT_SCATTER_FOR_N_IMPL(N == 10)
{
    LOG_FUNC("ten");
    F(0, 5), F(1, 6), F(2, 7), F(3, 8), F(4, 9);
    X(0, 3), X(1, 4), X(5, 8), X(6, 9);
    X(0, 2), X(3, 6), X(7, 9);
    X(0, 1), X(2, 4), X(5, 7), X(8, 9);
    X(1, 2), X(3, 5), X(4, 6), X(7, 8);
    X(1, 3), X(2, 5), X(4, 7), X(6, 8);
    X(2, 3), X(3, 4), X(6, 7), X(5, 6), X(4, 5);
}

SORT_SCATTER_FOR_N_IMPL(N == 12)
{
    LOG_FUNC("twelve");
    sort_scatter<4, NS, bias<B>(0 * NS)>(d + 0 * NS);
    sort_scatter<4, NS, bias<B>(4 * NS)>(d + 4 * NS);
    sort_scatter<4, NS, bias<B>(8 * NS)>(d + 8 * NS);
    X(1, 5), X(5, 9), X(1, 5), X(6, 10), X(2, 6), X(6, 10);
    X(0, 4), X(4, 8), X(0, 4), X(7, 11), X(3, 7), X(7, 11);
    X(1, 4), X(7, 10), X(3, 8);
    X(2, 3), X(2, 4), X(3, 5), X(8, 9), X(7, 9), X(6, 8);
    X(3, 4), X(5, 6), X(7, 8);
}

template <typename T, size_t NTotal>
template <size_t N, size_t NS, ptrdiff_t B>
void finite_sort<T, NTotal>::sort_scatter(T *d, fancy_choice<0>)
{
    LOG_FUNC("odd-even");

    constexpr auto Q = N / 2;
    constexpr auto P = N - Q;

    sort_scatter<P, 1 * NS, bias<B>(0 * NS)>(d + 0 * NS);
    sort_scatter<Q, 1 * NS, bias<B>(P * NS)>(d + P * NS);

    sort_scatter<P, 2 * NS, bias<P * NS>(0 * NS)>(d + 0 * NS);
    sort_scatter<Q, 2 * NS, bias<P * NS>(1 * NS)>(d + 1 * NS);

    for (size_t i = 1; i < N - 1; i += 2)
        X(i, i + 1);
}

#undef X
#undef F

template <typename T, size_t NTotal>
inline void finite_sort<T, NTotal>::sort(T *d)
{
#ifdef FINITE_SORT_DEBUG
    _base = d;
#endif
    sort_scatter<NTotal, 1>(d);
#ifdef FINITE_SORT_DEBUG
    std::cerr << "Total = " << _count << std::endl;
#endif
}
