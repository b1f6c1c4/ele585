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

        inline ~timed()
        {
            auto stop = std::chrono::high_resolution_clock::now();
            std::cerr << std::chrono::duration_cast<std::chrono::nanoseconds>(
                    stop - start
                    ).count() << std::endl;
        }

    private:
        std::chrono::high_resolution_clock::time_point start;
};

