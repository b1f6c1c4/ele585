#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdint>
#include "bitonic_playground.hpp"

int main()
{
    auto st = storage<size_t>{};
    st.nmach = 1;
    st.nmem = 64;
    st.nsec = 1;
    st.nmsg = 1;

    st.data.resize(st.nmach);
    st.data[0] = { 7, 4, 1, 3, 4, 6, 8, 5, 1, 5, 7, 4, 3, 8, 9, 4,
                   5, 4, 1, 3, 4, 5, 8, 9, 1, 5, 7, 4, 3, 8, 9, 4,
                   5, 4, 1, 3, 4, 5, 8, 9, 1, 5, 7, 4, 3, 8, 9, 4,
                   7, 4, 1, 3, 4, 6, 8, 5, 1, 5, 7, 4, 3, 8, 9, 4 };
    // st.data[0] = { 7, 4, 1, 3, 4, 6, 8, 9 };
    // st.data[1] = { 5, 2, 5, 8, 3, 7, 6, 9 };
    // st.data[2] = { 1, 3, 2, 4, 7, 3, 8, 3 };
    // st.data[3] = { 3, 4, 2, 7, 4, 6, 9, 2 };

    decltype(auto) bi = bitonic_remote_playground<size_t>(&st);
    bi.execute();

    for (size_t i = 0; i < st.nmach; i++)
    {
        std::cout << "Final[" << i << "]=";
        for (size_t j = 0; j < st.nmem * st.nsec; j++)
            std::cout << " " << st.data[i][j];
        std::cout << std::endl;
    }

    return 0;
}
