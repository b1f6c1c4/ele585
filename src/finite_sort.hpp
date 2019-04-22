#include <utility>

template <typename T>
inline void sort_unit(T &l, T& r)
{
    // auto lv = (reinterpret_cast<char *>(&l) - ptr) / sizeof(T);
    // auto rv = (reinterpret_cast<char *>(&r) - ptr) / sizeof(T);
    std::cerr << "[" << lv << "]--[" << rv << "]" << std::endl;

    if (l > r)
        std::swap(l, r);
}

template <typename T, size_t N, size_t NS>
inline typename std::enable_if<(N < 2), void>::type
sort_scatter(T *d) { }

template <typename T, size_t N, size_t NS>
inline typename std::enable_if<(N == 2), void>::type
sort_scatter(T *d)
{
    sort_unit<T>(d[0], d[NS]);
}

template <typename T, size_t N, size_t NS>
inline typename std::enable_if<(N > 2), void>::type
sort_scatter(T *d)
{
    sort_scatter<T, N - (N / 2), NS>(d);
    sort_scatter<T, N / 2, NS>(d + NS * (N - (N / 2)));
    sort_scatter<T, N - (N / 2), NS * 2>(d);
    sort_scatter<T, N / 2, NS * 2>(d + NS);
    for (size_t i = NS; i + NS < NS * N; i += NS * 2)
        sort_unit<T>(d[i], d[i + NS]);
}

template <typename T, size_t N>
inline void finite_sort(T *d)
{
    // ptr = reinterpret_cast<char *>(d);
    sort_scatter<T, N, 1>(d);
}
