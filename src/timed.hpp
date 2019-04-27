#pragma once

#include <iostream>
#include <chrono>

class timed final
{
    public:
        inline timed()
        {
            start = std::chrono::high_resolution_clock::now();
        }

        timed(const timed &) = delete;
        timed(timed &&) = delete;
        timed &operator=(const timed &) = delete;
        timed &operator=(timed &&) = delete;

		uint64_t done()
        {
            auto stop = std::chrono::high_resolution_clock::now();
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                    stop - start
                    ).count();
        }

    private:
        std::chrono::high_resolution_clock::time_point start;
};

