#pragma once

#include <vector>
#include <map>
#include <queue>
#include <thread>
#include <memory>
#include <condition_variable>
#include "bitonic_remote.hpp"

template <typename T>
struct storage
{
    size_t nmach;
    size_t nmem;
    size_t nsec;
    size_t nmsg;

    std::vector<std::vector<T>> data;
};

template <typename T>
class bitonic_remote_playground
{
    class Channel;
    class stub;

public:
    bitonic_remote_playground(storage<T> *st)
    {
        for (size_t i = 0; i < st->nmach; i++)
            _stubs.emplace_back(i, st, _mu0, &_queues);
        for (size_t i = 0; i < st->nmach; i++)
        {
            _queues.emplace_back();
            for (size_t j = 0; j < st->nmach; j++)
                _queues[i].emplace_back(std::make_shared<Channel>());
        }
    }

    void execute()
    {
        std::vector<std::thread> ths;
        for (size_t i = 0; i < _stubs.size(); i++)
            ths.emplace_back(&stub::execute, &_stubs[i]);
        for (decltype(auto) th : ths)
            th.join();
    }

private:
    class Channel
    {
        typedef typename bitonic_remote<T>::tag_t tag_t;

        std::mutex _mu;
        std::condition_variable _cv;
        std::map<tag_t, std::queue<std::vector<T>>> _qs;

    public:
        void send(const T *d, size_t sz, tag_t tag)
        {
            {
                std::lock_guard<std::mutex> l(_mu);
                _qs[tag].push(std::vector<T>(d, d + sz));
            }
            _cv.notify_one();
        }

        void recv(T *d, size_t sz, tag_t tag)
        {
            std::unique_lock<std::mutex> l(_mu);
            _cv.wait(l, [tag, this]{ return !_qs[tag].empty(); });
            decltype(auto) recv = _qs[tag].front();
            if (recv.size() != sz)
                throw;
            std::copy(recv.begin(), recv.end(), d);
            _qs[tag].pop();
        }
    };

    std::mutex _mu0;
    std::vector<stub> _stubs;
    std::vector<std::vector<std::shared_ptr<Channel>>> _queues;

    class stub : protected bitonic_remote<T>
    {
        size_t _my;
        std::mutex &_mu0;
        storage<T> *_st;
        std::vector<std::vector<std::shared_ptr<Channel>>> *_queues;
    public:
        stub(size_t my, storage<T> *st, std::mutex &mu0, std::vector<std::vector<std::shared_ptr<Channel>>> *queues)
            : bitonic_remote<T>(st->nmach, st->nmem, st->nsec, st->nmsg, new T[st->nmem]),
              _my(my), _mu0(mu0), _st(st), _queues(queues) { }

        void execute()
        {
            if (_st->nsec == 1)
            {
                decltype(auto) d = _st->data[_my];
                std::copy(d.begin(), d.end(), bitonic_remote<T>::_d);
            }
            bitonic_remote<T>::execute(_my);
            if (_st->nsec == 1)
            {
                decltype(auto) d = bitonic_remote<T>::_d;
                std::copy(d, d + _st->nmem, _st->data[_my].begin());
            }
        }

        stub(const stub &) = delete;
        stub(stub &&other) noexcept
            : bitonic_remote<T>(std::move(other)),
              _my(other._my), _mu0(other._mu0), _st(other._st),
              _queues(std::move(other._queues)) { }

        stub &operator=(const stub &) = delete;
        stub &operator=(stub &&other) noexcept
        {
            delete [] bitonic_remote<T>::_d;
            bitonic_remote<T>::_d = nullptr;

            bitonic_remote<T>::operator=(std::move(other));

            _my = other._my;
            _mu0 = other._mu0;
            _st = other._st;
            _queues = std::move(other._queues);

            return *this;
        }

        ~stub()
        {
            delete [] bitonic_remote<T>::_d;
            bitonic_remote<T>::_d = nullptr;
        }

    protected:
        typedef typename bitonic_remote<T>::tag_t tag_t;

        virtual void exchange_mem(
            size_t sz, size_t partner, tag_t tag,
            const T *source, T *dest) override
        {
            {
                std::lock_guard<std::mutex> l{_mu0};
                std::cout
                    << bitonic_remote<T>::My << " -> " << partner
                    << " (#" << std::hex << tag << std::dec << "):";
                for (size_t i = 0; i < sz; i++)
                    std::cout << " " << source[i];
                std::cout << std::endl;
            }

            (*_queues)[bitonic_remote<T>::My][partner]->send(source, sz, tag);
            (*_queues)[partner][bitonic_remote<T>::My]->recv(dest, sz, tag);

            {
                std::lock_guard<std::mutex> l{_mu0};
                std::cout
                    << bitonic_remote<T>::My << " <- " << partner
                    << " (#" << std::hex << tag << std::dec << "):";
                for (size_t i = 0; i < sz; i++)
                    std::cout << " " << dest[i];
                std::cout << std::endl;
            }
        }

        virtual void load_sec(size_t sec, size_t offset, T *d, size_t sz) override
        {
            auto base = _st->data[bitonic_remote<T>::My].begin()
                + bitonic_remote<T>::NMem * sec + offset;
            std::copy(base, base + sz, d);

            {
                std::lock_guard<std::mutex> l{_mu0};
                std::cout
                    << bitonic_remote<T>::My << " RD "
                    << "#" << sec << "[" << offset << "]:";
                for (size_t i = 0; i < sz; i++)
                    std::cout << " " << d[i];
                std::cout << std::endl;
            }
        }

        virtual void write_sec(size_t sec, size_t offset, const T *d, size_t sz) override
        {
            auto base = _st->data[bitonic_remote<T>::My].begin()
                + bitonic_remote<T>::NMem * sec + offset;
            std::copy(d, d + sz, base);

            {
                std::lock_guard<std::mutex> l{_mu0};
                std::cout
                    << bitonic_remote<T>::My << " WR "
                    << "#" << sec << "[" << offset << "]:";
                for (size_t i = 0; i < sz; i++)
                    std::cout << " " << d[i];
                std::cout << std::endl;
                std::cout << bitonic_remote<T>::My << " : ";
                for (decltype(auto) v : _st->data[bitonic_remote<T>::My])
                    std::cout << " " << v;
                std::cout << std::endl;
            }
        }
    };
};
