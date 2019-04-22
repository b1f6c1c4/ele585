#include <utility>

template <typename T, size_t NTotal, bool Debug = false>
class finite_sort
{
private:
    inline void sort_unit(T &l, T& r)
    {
        if (Debug)
        {
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
            std::cerr << "[" << lv << "]--[" << rv << "]" << std::endl;
            _count++;
        }

        if (l > r)
            std::swap(l, r);
    }

    template <size_t N, size_t NS, ptrdiff_t B>
        inline typename std::enable_if<(B < 0 || N * NS - NS + 1 <= B), void>::type
        sort_scatter(T *d) { }

    template <size_t N, size_t NS>
        void sort_scatter(T *d)
        {
            // std::cerr << "sort_scatter<" << N << ", " << NS << ">(d[" << d - _base << "])" << std::endl;
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

    template <size_t N, size_t NS, ptrdiff_t B>
        inline typename std::enable_if<!(B < 0 || N * NS - NS + 1 <= B), void>::type
        sort_scatter(T *d)
        {
            if (N < 2)
                return;
            // std::cerr << "sort_scatter<" << N << ", " << NS << ", " << B << ">(d[" << d - _base << "])" << std::endl;
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

    const T *_base;
    size_t _count;
public:
    inline void sort(T *d)
    {
        if (Debug)
            _base = d;
        sort_scatter<NTotal, 1>(d);
        if (Debug)
            std::cerr << "Total = " << _count << std::endl;
    }
};
