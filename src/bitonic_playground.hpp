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
    size_t nmsg;

    std::vector<std::vector<T>> data;
};

template <typename T>
class bitonic_remote_playground
{
    typedef bitonic_remote<T, 8, 2> bitonic;
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
        typedef typename bitonic::tag_t tag_t;

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

    class stub : protected bitonic
    {
        size_t _my;
        std::mutex &_mu0;
        storage<T> *_st;
        std::vector<std::vector<std::shared_ptr<Channel>>> *_queues;
    public:
        stub(size_t my, storage<T> *st, std::mutex &mu0, std::vector<std::vector<std::shared_ptr<Channel>>> *queues)
            : bitonic(st->nmach, st->nmem, st->nmsg, new T[st->nmem]),
              _my(my), _mu0(mu0), _st(st), _queues(queues) { }

        void execute()
        {
            {
                decltype(auto) d = _st->data[_my];
                std::copy(d.begin(), d.end(), bitonic::_d);
            }
            bitonic::execute(_my);
            {
                decltype(auto) d = bitonic::_d;
                std::copy(d, d + _st->nmem, _st->data[_my].begin());
            }
        }

        stub(const stub &) = delete;
        stub(stub &&other) noexcept
            : bitonic(std::move(other)),
              _my(other._my), _mu0(other._mu0), _st(other._st),
              _queues(std::move(other._queues)) { }

        stub &operator=(const stub &) = delete;
        stub &operator=(stub &&other) noexcept
        {
            delete [] bitonic::_d;
            bitonic::_d = nullptr;

            bitonic::operator=(std::move(other));

            _my = other._my;
            _mu0 = other._mu0;
            _st = other._st;
            _queues = std::move(other._queues);

            return *this;
        }

        ~stub()
        {
            delete [] bitonic::_d;
            bitonic::_d = nullptr;
        }

    protected:
        typedef typename bitonic::tag_t tag_t;

        virtual void exchange_mem(
            size_t sz, size_t partner, tag_t tag,
            const T *source, T *dest) override
        {
            {
                std::lock_guard<std::mutex> l{_mu0};
                std::cout
                    << bitonic::My << " -> " << partner
                    << " (#" << std::hex << tag << std::dec << "):";
                for (size_t i = 0; i < sz; i++)
                    std::cout << " " << source[i];
                std::cout << std::endl;
            }

            (*_queues)[bitonic::My][partner]->send(source, sz, tag);
            (*_queues)[partner][bitonic::My]->recv(dest, sz, tag);

            {
                std::lock_guard<std::mutex> l{_mu0};
                std::cout
                    << bitonic::My << " <- " << partner
                    << " (#" << std::hex << tag << std::dec << "):";
                for (size_t i = 0; i < sz; i++)
                    std::cout << " " << dest[i];
                std::cout << std::endl;
            }
        }
    };
};
