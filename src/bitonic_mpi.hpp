#pragma once

#include <fstream>
#include <mpi.h>
#include "bitonic_remote.hpp"

template <typename T>
class bitonic_remote_mpi : public bitonic_remote<T>
{
private:
    typedef typename bitonic_remote<T>::tag_t tag_t;

public:
    bitonic_remote_mpi(size_t nmach, size_t nmem, size_t nsec, const std::string &fn)
        : bitonic_remote<T>(nmach, nmem, nsec), _f(fn, std::ios::binary) { }

protected:
    virtual void send_mem(const T *d, size_t sz, size_t partner, tag_t tag) override
    {
        std::cerr
            << bitonic_remote<T>::My << " -> " << partner
            << " (#" << std::hex << tag << ") "
            << sz << std::endl;
        MPI_Send(
                reinterpret_cast<const void *>(d), sz * sizeof(T),
                MPI_UNSIGNED_CHAR, partner, tag, MPI_COMM_WORLD);
    }

    virtual void recv_mem(T *d, size_t sz, size_t partner, tag_t tag) override
    {
        std::cerr
            << bitonic_remote<T>::My << " <- " << partner
            << " (" << std::hex << tag << ") "
            << sz << std::endl;
        MPI_Recv(
                reinterpret_cast<void *>(d), sz * sizeof(T),
                MPI_UNSIGNED_CHAR, partner, tag, MPI_COMM_WORLD, nullptr);
    }

    virtual void load_sec(size_t sec, size_t offset, T *d, size_t sz) override
    {
        std::cout
            << bitonic_remote<T>::My << " RD "
            << "#" << sec << "[" << offset << "] "
            << sz << std::endl;
        _f.seekg(bitonic_remote<T>::NMem * sec + offset, std::ios::beg);
        _f.read(reinterpret_cast<char *>(d), sizeof(T) * sz);
    }

    virtual void write_sec(size_t sec, size_t offset, const T *d, size_t sz) override
    {
        std::cout
            << bitonic_remote<T>::My << " WR "
            << "#" << sec << "[" << offset << "] "
            << sz << std::endl;
        _f.seekg(bitonic_remote<T>::NMem * sec + offset, std::ios::beg);
        _f.write(reinterpret_cast<const char *>(d), sizeof(T) * sz);
    }

    std::fstream _f;
};
