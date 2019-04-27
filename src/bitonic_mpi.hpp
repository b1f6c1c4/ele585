#pragma once

#include <fstream>
#include <stdexcept>
#include <mpi.h>
#include "bitonic_remote.hpp"

// #define BITONIC_MPI_DEBUG
// #define BITONIC_MPI_TRACE

template <typename T>
class bitonic_remote_mpi : public bitonic_remote<T>
{
private:
    typedef typename bitonic_remote<T>::tag_t tag_t;

public:
    bitonic_remote_mpi(size_t nmach, size_t nmem, size_t nsec, size_t nmsg, const std::string &fn)
        : bitonic_remote<T>(nmach, nmem, nsec, nmsg),
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
            << bitonic_remote<T>::My << " -> " << partner
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
            << bitonic_remote<T>::My << " <- " << partner
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

    virtual void load_sec(size_t sec, size_t offset, T *d, size_t sz) override
    {
        const auto seek = (bitonic_remote<T>::NMem * sec + offset) * sizeof(T);
        if (!_f.seekg(seek, _f.beg))
        {
            std::cout << "Can't seek file for read" << std::endl;
            throw std::runtime_error("Can't seek file for read");
        }
        if (!_f.read(reinterpret_cast<char *>(d), sizeof(T) * sz))
        {
            std::cout << "Can't read file" << std::endl;
            throw std::runtime_error("Can't read file");
        }

#ifdef BITONIC_MPI_DEBUG
        std::cout
            << bitonic_remote<T>::My << " RD "
            << "#" << sec << "[" << offset << "] ";
#ifdef BITONIC_MPI_TRACE
        for (size_t i = 0; i < sz; i++)
            std::cout << " " << d[i];
        std::cout << std::endl;
#else
        std::cout << sz << std::endl;
#endif
#endif
    }

    virtual void write_sec(size_t sec, size_t offset, const T *d, size_t sz) override
    {
#ifdef BITONIC_MPI_DEBUG
        std::cout
            << bitonic_remote<T>::My << " WR "
            << "#" << sec << "[" << offset << "] ";
#ifdef BITONIC_MPI_TRACE
        for (size_t i = 0; i < sz; i++)
            std::cout << " " << d[i];
        std::cout << std::endl;
#else
        std::cout << sz << std::endl;
#endif
#endif

        const auto seek = (bitonic_remote<T>::NMem * sec + offset) * sizeof(T);
        if (!_f.seekg(seek, _f.beg))
            throw std::runtime_error("Can't seek file for read");
        if (!_f.write(reinterpret_cast<const char *>(d), sizeof(T) * sz))
            throw std::runtime_error("Can't write file");
    }
};
