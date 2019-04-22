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
        sort_scatter<N, NS, B>(d, fancy_choice<2>{});
    }

    template <size_t N, size_t NS, ptrdiff_t B = INVALID,
             typename std::enable_if_t<
                 (N < 2 || B != INVALID && (B <= 0 || N * NS - NS + 1 <= B))
                 >* = nullptr>
    inline void sort_scatter(T *d, fancy_choice<2>)
    {
        LOG_FUNC("trivial");
    }

#define SORT_SCATTER_FOR_N_DECL(x) \
    template <size_t N, size_t NS, ptrdiff_t B = INVALID, \
             typename std::enable_if_t<N == x>* = nullptr> \
    inline void sort_scatter(T *d, fancy_choice<1>)

#define SORT_SCATTER_FOR_N_IMPL(x) \
    template <typename T, size_t NTotal> \
    template <size_t N, size_t NS, ptrdiff_t B, \
             typename std::enable_if_t<N == x>*> \
    inline void finite_sort<T, NTotal>::sort_scatter(T *d, fancy_choice<1>)

    SORT_SCATTER_FOR_N_DECL(2);
    SORT_SCATTER_FOR_N_DECL(3);
    SORT_SCATTER_FOR_N_DECL(6);
    SORT_SCATTER_FOR_N_DECL(9);
    SORT_SCATTER_FOR_N_DECL(10);
    SORT_SCATTER_FOR_N_DECL(12);
    SORT_SCATTER_FOR_N_DECL(13);
    SORT_SCATTER_FOR_N_DECL(16);

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

#define UNIT(l, r) sort_scatter<2, (r - l) * NS, bias<B>(l * NS)>(d + l * NS)

SORT_SCATTER_FOR_N_IMPL(2)
{
    LOG_FUNC("two");
    sort_unit(d[0], d[NS]);
}

SORT_SCATTER_FOR_N_IMPL(3)
{
    LOG_FUNC("three");
    UNIT(0, 1), UNIT(1, 2), UNIT(0, 1);
}

SORT_SCATTER_FOR_N_IMPL(9)
{
    LOG_FUNC("nine");
    UNIT(0, 1), UNIT(1, 2), UNIT(0, 1);
    UNIT(3, 4), UNIT(4, 5), UNIT(3, 4);
    UNIT(6, 7), UNIT(7, 8), UNIT(6, 7);
    UNIT(0, 3), UNIT(3, 6), UNIT(0, 3);
    UNIT(1, 4), UNIT(4, 7), UNIT(1, 4);
    UNIT(2, 5), UNIT(5, 8), UNIT(2, 5);
    UNIT(1, 3), UNIT(5, 7);
    UNIT(2, 6), UNIT(4, 6), UNIT(2, 4);
    UNIT(2, 3), UNIT(5, 6);
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

    // Don't use UNIT here because B no longer holds
    for (size_t i = NS; i + NS < NS * N; i += NS * 2)
        sort_unit(d[i], d[i + NS]);
}

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
