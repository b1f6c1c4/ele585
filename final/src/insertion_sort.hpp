#include <utility>

template <typename Iter>
void insertion_sort(Iter first, Iter last)
{
    for (auto i = first + 1; i != last; i++)
        for (auto j = i; j != first; j--)
            if (*j < *(j - 1))
                std::swap(*j, *(j - 1));
            else
                break;
}
