#include <utility>
#include <limits>
#include "fancy_choice.hpp"

// #define FINITE_SORT_DEBUG
#ifdef FINITE_SORT_DEBUG_EX
#define LOG_FUNC(str) do { \
        std::cerr << "sort_scatter<" << N << ", " << NS << ", "; \
        if (BR != INVALID) \
            std::cerr << BR; \
        std::cerr << ">(d[" << B << "]) " str << std::endl; \
    } while (false)
#else
#define LOG_FUNC
#endif

#define INVALID PTRDIFF_MAX

template <typename T, size_t NTotal>
class finite_sort
{
private:
    inline void sort_unit(T &l, T& r);

    template <size_t N, size_t NS, size_t B = 0, ptrdiff_t BR = INVALID>
    inline void sort_scatter(T *d)
    {
        sort_scatter<N, NS, B, BR>(d, fancy_choice<4>{});
    }

    template <size_t N, size_t NS, size_t B = 0, ptrdiff_t BR = INVALID,
             typename std::enable_if_t<
                 (N < 2 || BR != INVALID && (BR <= B || B + N * NS + 1 <= BR + NS))
                 >* = nullptr>
    inline void sort_scatter(T *d, fancy_choice<4>)
    {
        LOG_FUNC("trivial");
    }

    template <size_t N, size_t NS, size_t B = 0, ptrdiff_t BR = INVALID,
             typename std::enable_if_t<
                 (!(B + N * NS - NS + 1 <= NTotal) && N <= 10)
                 >* = nullptr>
    inline void sort_scatter(T *d, fancy_choice<3>)
    {
        LOG_FUNC("downgrade");
        const auto REST = NTotal - B;
        const auto NX = ((REST > NS * (REST / NS)) ? 1 : 0) + REST / NS;
        sort_scatter<NX, NS, B, BR>(d);
    }

#define SORT_SCATTER_FOR_N_DECL(x) \
    template <size_t N, size_t NS, size_t B = 0, ptrdiff_t BR = INVALID, \
             typename std::enable_if_t<(x)>* = nullptr> \
    inline void sort_scatter(T *d, fancy_choice<2>)

#define SORT_SCATTER_FOR_N_IMPL(x) \
    template <typename T, size_t NTotal> \
    template <size_t N, size_t NS, size_t B, ptrdiff_t BR, \
             typename std::enable_if_t<(x)>*> \
    inline void finite_sort<T, NTotal>::sort_scatter(T *d, fancy_choice<2>)

    SORT_SCATTER_FOR_N_DECL(N == 2);
    SORT_SCATTER_FOR_N_DECL(N == 3);
    SORT_SCATTER_FOR_N_DECL(N == 9);
    SORT_SCATTER_FOR_N_DECL(N == 10);
    SORT_SCATTER_FOR_N_DECL(N == 12);
    SORT_SCATTER_FOR_N_DECL(N == 13);
    SORT_SCATTER_FOR_N_DECL(N == 16);

    template <size_t N, size_t NS, size_t B = 0, ptrdiff_t BR = INVALID, \
             typename std::enable_if_t<(N > 10 && N < 20)>* = nullptr> \
    inline void sort_scatter(T *d, fancy_choice<1>)
    {
        LOG_FUNC("upgrade");
        sort_scatter<N + 1, NS, B, BR>(d);
    }

    template <size_t N, size_t NS, size_t B = 0, ptrdiff_t BR = INVALID>
    inline void sort_scatter(T *d, fancy_choice<0>);

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

#define F(l, r) sort_scatter<2, ((r) - (l)) * NS, B + (l) * NS, BR>(d)
#define X(l, r) sort_scatter<2, ((r) - (l)) * NS, B + (l) * NS>(d)

SORT_SCATTER_FOR_N_IMPL(N == 2)
{
    LOG_FUNC("two");
    if (B + NS < NTotal)
        sort_unit(d[B], d[B + NS]);
}

SORT_SCATTER_FOR_N_IMPL(N == 3)
{
    LOG_FUNC("three");
    if (BR > B + NS)
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
    sort_scatter<4, NS, B + (0 * NS), BR>(d);
    sort_scatter<4, NS, B + (4 * NS), BR>(d);
    sort_scatter<4, NS, B + (8 * NS), BR>(d);
    X(1, 5), X(5, 9), X(1, 5), X(6, 10), X(2, 6), X(6, 10);
    X(0, 4), X(4, 8), X(0, 4), X(7, 11), X(3, 7), X(7, 11);
    X(1, 4), X(7, 10), X(3, 8);
    X(2, 3), X(2, 4), X(3, 5), X(8, 9), X(7, 9), X(6, 8);
    X(3, 4), X(5, 6), X(7, 8);
}

SORT_SCATTER_FOR_N_IMPL(N == 13)
{
    LOG_FUNC("thirteen");
    F(0, 5), F(1, 7), F(3, 9), F(2, 4), F(6, 11), F(8, 12);
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

SORT_SCATTER_FOR_N_IMPL(N == 16)
{
    LOG_FUNC("sixteen");
#define STAGE(x) F(x + 0, x + 1), F(x + 2, x + 3), X(x + 0, x + 2), X(x + 1, x + 3)
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

template <typename T, size_t NTotal>
template <size_t N, size_t NS, size_t B, ptrdiff_t BR>
void finite_sort<T, NTotal>::sort_scatter(T *d, fancy_choice<0>)
{
    LOG_FUNC("odd-even");

    constexpr auto Q = N / 2;
    constexpr auto P = N - Q;

    sort_scatter<P, 1 * NS, B + (0 * NS), BR>(d);
    sort_scatter<Q, 1 * NS, B + (P * NS), BR>(d);

    sort_scatter<P, 2 * NS, B + (0 * NS), B + P * NS>(d);
    sort_scatter<Q, 2 * NS, B + (1 * NS), B + P * NS>(d);

    for (size_t i = 1; i < N - 1; i += 2)
        if (B + i * NS + NS < NTotal)
            sort_unit(d[B + i * NS], d[B + i * NS + NS]);
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
