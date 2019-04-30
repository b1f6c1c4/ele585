#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdint>
#include "bitonic_playground.hpp"

int main()
{
    auto st = storage<size_t>{};
    st.nmach = 4;
    st.nmem = 16;
    st.nmsg = 32;

    st.data.resize(st.nmach);
    st.data[0] = { 7, 4, 1, 3, 4, 6, 8, 5, 1, 5, 7, 4, 3, 8, 9, 4 };
    st.data[1] = { 2, 6, 3, 5, 8, 8, 5, 2, 6, 9, 5, 3, 6, 8, 7, 1 };
    st.data[2] = { 7, 4, 1, 3, 4, 6, 8, 5, 1, 5, 7, 4, 3, 8, 9, 4 };
    st.data[3] = { 2, 6, 3, 5, 8, 8, 5, 2, 6, 9, 5, 3, 6, 8, 7, 1 };
    // st.data[0] = { 7, 4, 1, 3, 4, 6, 8, 9 };
    // st.data[1] = { 5, 2, 5, 8, 3, 7, 6, 9 };
    // st.data[2] = { 1, 3, 2, 4, 7, 3, 8, 3 };
    // st.data[3] = { 3, 4, 2, 7, 4, 6, 9, 2 };

    bitonic_remote_playground<size_t> bi(&st);
    bi.execute();

    for (size_t i = 0; i < st.nmach; i++)
    {
        std::cout << "Final[" << i << "]=";
        for (size_t j = 0; j < st.nmem; j++)
            std::cout << " " << st.data[i][j];
        std::cout << std::endl;
    }

    return 0;
}
