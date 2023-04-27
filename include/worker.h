//
// Copyright 2023 Suzuki Yoshinori(wave.suzuki.z@gmail.com)
// since: 21 Apr 2023
//
#pragma once

#include "atomic_queue.h"
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <thread>
#include <vector>

namespace SimpleWorker
{
using count_t = std::atomic_int32_t;

// 単独ワーカー
class Worker
{
  public:
    virtual ~Worker()  = default;
    virtual void run() = 0;
};

// Worker Thread
class WorkerThread
{
  private:
    using aint64 = std::atomic_int64_t;

    // 依存性のない単独ワーカー
    class WorkerSlot
    {
        Worker*     worker    = nullptr;
        count_t*    counter   = nullptr;
        count_t*    reference = nullptr;
        const char* name      = nullptr;

      public:
        void set(Worker* w, count_t* c, count_t* r, const char* n = nullptr)
        {
            worker    = w;
            counter   = c;
            reference = r;
            name      = n;
        }
        void        run() { worker->run(); }
        count_t*    getCounter() { return counter; }
        count_t*    getReference() { return reference; }
        const char* getName() const { return name; }
    };

    std::vector<std::thread> threadList;
    std::mutex               executeMutex{};
    std::condition_variable  executeCondition{};
    std::condition_variable  waitCondition{};
    std::atomic_bool         enabled{false};
    std::atomic_int          executeCount{0};
    Queue<WorkerSlot>        workerQueue;
    Queue<WorkerSlot>        stockQueue;
    std::vector<WorkerSlot>  workerSlot;

    // ワーカー実行
    void update()
    {
        for (;;)
        {
            {
                // 実行開始待ち
                std::unique_lock lk(executeMutex);
                while (enabled && workerQueue.empty())
                {
                    executeCondition.wait(lk);
                }
            }
            if (enabled == false)
            {
                // 実行終了
                break;
            }

            // コンシューマ実行カウンタ(=スレッド数)
            executeCount.fetch_add(1);
            while (auto* wslot = workerQueue.pop())
            {
                auto* reference = wslot->getReference();
                if (reference == nullptr || reference->load() <= 0)
                {
                    // リファレンスカウンタの状態を見てワーカー実行
                    wslot->run();
                    if (auto* counter = wslot->getCounter())
                    {
                        // カウンタが設定されていたら減算
                        counter->fetch_sub(1);
                    }
                    // ストックに戻す
                    stockQueue.push(wslot);
                }
                else
                {
                    // 今回実行しなかったのでもう一度キューに積む
                    workerQueue.push(wslot);
                }
            }
            executeCount.fetch_sub(1);
            if (executeCount.load() == 0 && workerQueue.empty())
            {
                // 全ての実行終了
                waitCondition.notify_all();
            }
        }
    }

  public:
    //
    WorkerThread(const WorkerThread&)            = delete;
    WorkerThread& operator=(const WorkerThread&) = delete;
    //
    WorkerThread(size_t nbWorker = 2000, size_t nbThread = 3) : workerQueue(nbWorker + 1), stockQueue(nbWorker + 1)
    {
        enabled = true;
        threadList.resize(nbThread);
        for (auto& th : threadList)
        {
            th = std::thread([this] { update(); });
        }
        // ワーカー用スロットを確保して待機用キューに全て積んでおく
        workerSlot.resize(nbWorker);
        for (size_t i = 0; i < nbWorker; i++)
        {
            stockQueue.push(&workerSlot[i]);
        }
    }

    //
    virtual ~WorkerThread() { stop(); }

    //
    void stop()
    {
        enabled = false;
        execute();
        for (auto& th : threadList)
        {
            th.join();
        }
    }

    //
    void execute() { executeCondition.notify_all(); }

    //
    void wait()
    {
        std::unique_lock<std::mutex> lk(executeMutex);
        waitCondition.wait(lk, [this] { return executeCount.load() == 0 && workerQueue.empty(); });
    }

    // ワーカーセット
    bool push(Worker& worker, count_t* cnt = nullptr, const char* name = nullptr)
    {
        if (auto* wslot = stockQueue.pop())
        {
            wslot->set(&worker, cnt, nullptr, name);
            workerQueue.push(wslot);
            return true;
        }
        return false;
    }
    // カウンタ依存ワーカーセット
    bool push(std::atomic_int* ref, Worker& worker, count_t* cnt = nullptr, const char* name = nullptr)
    {
        if (auto* wslot = stockQueue.pop())
        {
            wslot->set(&worker, cnt, ref, name);
            workerQueue.push(wslot);
            return true;
        }
        return false;
    }
};

//
// カウンタ共有して実行させるユーティリティ
//
class ChainWorker : public Worker
{
  private:
    WorkerThread* owner   = nullptr;
    count_t*      counter = nullptr;

  protected:
    WorkerThread* getOwner() { return owner; }
    count_t*      getCounter() { return counter; }

  public:
    // ワーカースレッドに繋いで実行させる
    // 登録したワーカーが動いている間に実行を開始するかもしれないので注意
    void attach(WorkerThread& wt, count_t* cnt)
    {
        owner   = &wt;
        counter = cnt;
        if (counter != nullptr)
        {
            counter->fetch_add(1);
            wt.push(*this, counter);
        }
    }
    // ワーカースレッドに繋いで実行させる
    // 登録したワーカーが動いている間に実行を開始するかもしれないので注意
    void attach(count_t* ref, WorkerThread& wt, count_t* cnt)
    {
        owner   = &wt;
        counter = cnt;
        if (counter != nullptr)
        {
            counter->fetch_add(1);
            wt.push(ref, *this, counter);
        }
    }
};

} // namespace SimpleWorker
//
