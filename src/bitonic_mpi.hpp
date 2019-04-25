#pragma once

#include <fstream>
#include "bitonic_remote.hpp"

template <typename T>
class bitonic_remote_mpi : public bitonic_remote<T>
{
private:
    typedef typename bitonic_remote<T>::tag_t tag_t;

protected:
    virtual void send_mem(const T *d, size_t sz, size_t partner, tag_t tag) override
    {
        // TODO
    }

    virtual void recv_mem(T *d, size_t sz, size_t partner, tag_t tag)override
    {
        // TODO
    }

    virtual void load_sec(size_t sec, size_t offset, T *d, size_t sz)override
    {
        _f.seekg(bitonic_remote<T>::NMem * sec + offset, std::ios::beg);
        _f.read(reinterpret_cast<char *>(d), sizeof(T) * sz);
    }
    virtual void write_sec(size_t sec, size_t offset, const T *d, size_t sz)override
    {
        _f.seekg(bitonic_remote<T>::NMem * sec + offset, std::ios::beg);
        _f.write(reinterpret_cast<char *>(d), sizeof(T) * sz);
    }

    std::fstream _f;

public:
    bitonic_remote_mpi(size_t nmach, size_t nmem, size_t nsec, const std::string &fn)
        : bitonic_remote<T>(nmach, nmem, nsec), _f(fn, std::ios::binary) { }
};
