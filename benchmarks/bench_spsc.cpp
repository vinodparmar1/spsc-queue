#include <thread>
#include <atomic>
#include <memory>
#include <benchmark/benchmark.h>

#include "ring_buffer.hpp"

static void BM_PushLatency(benchmark::State& state) {
    auto q = std::make_shared<spsc::SPSCQueue<int, 65536>>();
    std::atomic<bool> con_running{true};
    std::thread consumer([&]() {
        int val = 0;
        while(true) {
            if(!con_running.load(std::memory_order_relaxed)) {
                break;
            }
            q->pop(val);
        }
    });
    int i = 10;
    for(auto _: state) {
        q->push(i);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations());
    con_running.store(false, std::memory_order_relaxed);
    consumer.join();
}
BENCHMARK(BM_PushLatency);

static void  BM_PushPop(benchmark::State& state) {
    auto q = std::make_shared<spsc::SPSCQueue<int, 65536>>();
    int i = 10;
    for(auto _: state) {
        q->push(i);
        q->pop(i);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_PushPop);

static void BM_Throughput(benchmark::State& state) {
    auto q = std::make_shared<spsc::SPSCQueue<int, 65536>>();
    std::atomic<bool> running{true};
    std::thread consumer([&]() {
        int val;
        while(running.load(std::memory_order_relaxed)) {
            q->pop(val);
            // no yield
        }
    });
    int i = 0;
    for(auto _:state) {
        // spin until push succeeds
        while(!q->push(i)) {}
        benchmark::ClobberMemory();
        ++i;
    }
    state.SetItemsProcessed(state.iterations());
    running.store(false);
    consumer.join();
}
BENCHMARK(BM_Throughput);

template<typename T, size_t Capacity>
struct NaiveSPSC {
    static constexpr size_t mask = Capacity - 1;
    alignas(64) std::atomic<uint64_t> head{0};
    alignas(64) std::atomic<uint64_t> tail{0};
    alignas(64) std::array<T, Capacity> buf;

    bool push(const T& item) {
        auto t = tail.load(std::memory_order_relaxed);
        // Always read head - no caching
        if (t == head.load(std::memory_order_acquire) + Capacity)
            return false;
        buf[t & mask] = item;
        tail.store(t + 1, std::memory_order_release);
        return true;
    }
    bool pop(T& item) {
        auto h = head.load(std::memory_order_relaxed);
        // always read tail - no caching
        if (h == tail.load(std::memory_order_acquire))
            return false;
        item = buf[h & mask];
        head.store(h + 1, std::memory_order_release);
        return true;
    }
};

static void BM_PushNaive(benchmark::State& state) {
    auto q = std::make_shared<NaiveSPSC<int, 65536>>();
    std::atomic<bool> running{true};
    std::thread consumer([&]() {
        int val;
        while (running.load(std::memory_order_relaxed)) {
            if (!q->pop(val)) std::this_thread::yield();
        }
    });

    int i = 10;
    for (auto _ : state) {
        while(!q->push(i)) {};
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations());

    running.store(false);
    consumer.join();
}
BENCHMARK(BM_PushNaive);
BENCHMARK_MAIN();
