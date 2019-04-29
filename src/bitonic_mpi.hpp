#pragma once

#include <fstream>
#include <stdexcept>
#include <mpi.h>
#include "bitonic_remote.hpp"

// #define BITONIC_MPI_DEBUG
// #define BITONIC_MPI_TRACE

// Maximum size (bytes) of MPI small package
#define X_MPI_SMALL 2048
// If single trip small package latency is T,
// how many bytes can we send in T using huge package?
#define X_LAMBDA 1.7e-6 * 200e6

template <typename T>
class bitonic_remote_mpi
    : public bitonic_remote<T, X_MPI_SMALL, static_cast<size_t>(X_LAMBDA)>
{
private:
    typedef bitonic_remote<T, X_MPI_SMALL, static_cast<size_t>(X_LAMBDA)> bitonic;
    typedef typename bitonic::tag_t tag_t;

public:
    bitonic_remote_mpi(size_t nmach, size_t nmem, size_t nmsg, T *d)
        : bitonic(nmach, nmem, nmsg, d), _f() { }

    bitonic_remote_mpi(size_t nmach, size_t nmem, size_t nmsg, const std::string &fn, T *d)
        : bitonic(nmach, nmem, nmsg, d),
          _f(fn, std::ios::in | std::ios::out | std::ios::binary)
    {
        if (!_f.is_open())
            throw std::runtime_error("Can't open file");
    }

protected:
    std::fstream _f;

    virtual void exchange_mem(
        size_t sz, size_t partner, tag_t tag,
        const T *source, T *dest) override
    {
#ifdef BITONIC_MPI_DEBUG
        std::cout
            << bitonic::My << " -> " << partner
            << " (#" << std::hex << tag << std::dec << ") ";
#ifdef BITONIC_MPI_TRACE
        for (size_t i = 0; i < sz; i++)
            std::cout << " " << source[i];
        std::cout << std::endl;
#else
        std::cout << sz << std::endl;
#endif
#endif

        MPI_Sendrecv(
                reinterpret_cast<const void *>(source), sz * sizeof(T),
                MPI_UNSIGNED_CHAR, partner, tag,
                reinterpret_cast<void *>(dest), sz * sizeof(T),
                MPI_UNSIGNED_CHAR, partner, tag,
                MPI_COMM_WORLD, nullptr);

#ifdef BITONIC_MPI_DEBUG
        std::cout
            << bitonic::My << " <- " << partner
            << " (#" << std::hex << tag << std::dec << ") ";
#ifdef BITONIC_MPI_TRACE
        for (size_t i = 0; i < sz; i++)
            std::cout << " " << dest[i];
        std::cout << std::endl;
#else
        std::cout << sz << std::endl;
#endif
#endif
    }
};
