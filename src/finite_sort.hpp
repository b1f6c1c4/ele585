#include <utility>

template <typename T, size_t NTotal>
class finite_sort
{
private:
    inline void sort_unit(T &l, T& r);

    template <size_t N, size_t NS>
    void sort_scatter(T *d);

    template <size_t N, size_t NS, ptrdiff_t B>
    void sort_scatter(T *d);

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


template <typename T, size_t NTotal>
template <size_t N, size_t NS>
void finite_sort<T, NTotal>::sort_scatter(T *d)
{
#ifdef FINITE_SORT_DEBUG_EX
    std::cerr << "sort_scatter<" << N << ", " << NS << ">(d[" << d - _base << "])" << std::endl;
#endif
    if (N < 2)
        return;
    if (N == 2)
    {
        sort_unit(d[0], d[NS]);
        return;
    }
    constexpr auto Q = N / 2;
    constexpr auto P = N - Q;
    sort_scatter<P, NS>(d);
    sort_scatter<Q, NS>(d + NS * P);
    sort_scatter<P, NS * 2, P>(d);
    sort_scatter<Q, NS * 2, P - NS>(d + NS);
    for (size_t i = NS; i + NS < NS * N; i += NS * 2)
        sort_unit(d[i], d[i + NS]);
}

template <typename T, size_t NTotal>
template <size_t N, size_t NS, ptrdiff_t B>
void finite_sort<T, NTotal>::sort_scatter(T *d)
{
    if (N < 2)
        return;
    if (B < 0 || N * NS - NS + 1 <= B)
        return;
#ifdef FINITE_SORT_DEBUG_EX
    std::cerr << "sort_scatter<" << N << ", " << NS << ", " << B << ">(d[" << d - _base << "])" << std::endl;
#endif
    if (N == 2)
    {
        sort_unit(d[0], d[NS]);
        return;
    }
    constexpr auto Q = N / 2;
    constexpr auto P = N - Q;
    sort_scatter<P, NS, B>(d);
    sort_scatter<Q, NS, B>(d + NS * P);
    sort_scatter<P, NS * 2, P>(d);
    sort_scatter<Q, NS * 2, P - NS>(d + NS);
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
