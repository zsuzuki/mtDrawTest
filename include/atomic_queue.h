//
// Copyright 2018 Yoshinori Suzuki<wave.suzuki.z@gmail.com>
//
#pragma once

#include <atomic>
#include <cstddef>
#include <vector>

template <class T>
struct QueueValue
{
    std::atomic<T> value;

    //
    QueueValue() = default;
    QueueValue(T n) { value = n; }
    QueueValue(T& n) { value = n; }
    QueueValue(T&& n) { value = n; }

    //
    QueueValue<T>& operator=(const QueueValue<T>& other)
    {
        value = other.pointer;
        return *this;
    }
    QueueValue<T>& operator=(T other)
    {
        value = other;
        return *this;
    }
      operator T() { return value.load(); }
    T load() const { return value.load(); }
};

//
// atomic queue <pointer> class
//
template <class T>
class Queue
{
    std::vector<QueueValue<T*>> buffer;

    std::atomic_int start{0};
    std::atomic_int end{0};

  public:
    Queue(size_t sz) : buffer(sz)
    {
        for (auto& pt : buffer)
        {
            pt = nullptr;
        }
    }

    // バッファが満杯の状態でpushしないこと余裕を持ったバッファ量を心がける
    bool push(T* p)
    {
        for (;;)
        {
            // とにかく'e'の位置を確保しに行く
            int e = end.load();
            int n = (e + 1) % buffer.size();
            if (end.compare_exchange_weak(e, n))
            {
                buffer[e] = p;
                return true;
            }
        }
    }

    //
    T* pop()
    {
        int  s = start;
        auto p = buffer[s].load();
        if (p == nullptr)
        {
            // 取るものがない
            return nullptr;
        }

        int n = (s + 1) % buffer.size();
        if (start.compare_exchange_weak(s, n) == false)
        {
            // 争奪戦に負けた
            return nullptr;
        }
        buffer[s] = nullptr;
        return p;
    }
    bool empty() const { return start == end; }
};

//
// atomic queue <number> class
//
template <class T>
class NQueue
{
    std::vector<QueueValue<T>> buffer;
    std::atomic_int            start{0};
    std::atomic_int            end{0};
    T                          invalid_value;

  public:
    NQueue(size_t sz, T invalid = 0) : buffer(sz)
    {
        invalid_value = invalid;
        for (auto& pt : buffer)
        {
            pt = invalid;
        }
    }

    //
    bool push(T p)
    {
        int n, e = end;
        do
        {
            n = (e + 1) % buffer.size();
            if (n == start)
                return false;
        } while (end.compare_exchange_weak(e, n) == false);
        buffer[e] = p;
        return true;
    }

    //
    struct Result
    {
        T    value;
        bool ok;
        //
        operator bool() const { return ok; }
    };
    Result pop()
    {
        int s = start;
        if (s == end)
            return Result{invalid_value, false};
        int n = (s + 1) % buffer.size();
        if (start.compare_exchange_weak(s, n) == false)
            return Result{invalid_value, false};

        auto p = buffer[s].load();
        while (p == invalid_value)
            p = buffer[s];

        buffer[s] = invalid_value;
        return Result{p, true};
    }
    bool empty() const { return start == end; }
};
