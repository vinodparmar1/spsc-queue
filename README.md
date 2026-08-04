# SPSC queue - A header only - Single producer, single consumer queue library; implements a low-latency queue for inter thread communication using lock-free mechanism

## current status
- include wait strategies i.e., spin wait, sleep wait, yield wait
- usage examples
- benchmark

## pending items
- batch consumer; Batch pop (say batch of 32): pop_batch → process 32 items → pop_batch → process 32 items
- Block wait strategy

## Build

```bash
mkdir -p build && build
cmake ..
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --clean-first
```

## Usage 

```cpp
    spsc::SPSCQueue<int, 64> queue;
    spsc::YieldWait wait;
    std::atomic<bool> prod_running{true};
    std::atomic<uint64_t> push_count{0};
    std::atomic<uint64_t> pop_count{0};

    std::thread producer([&]() {
        while(true) {
            if(!prod_running.load(std::memory_order_relaxed)) {
                break;
            }
            if(!queue.push(5)) {
                wait.wait();
            }
            else {
                push_count.fetch_add(1, std::memory_order_relaxed);
            }
        }
    });
    std::atomic<bool> con_running{true};
    std::thread consumer([&]() {
        int val = 0;
        while(true) {
            if(!con_running.load(std::memory_order_relaxed)) {
                break;
            }
            if(!queue.pop(val)) {
                wait.wait();
            }
            else {
                pop_count.fetch_add(1, std::memory_order_relaxed);
            }
        }
    });
```
## Benchmark

```bash
2026-08-04T19:21:58+00:00
Running ./bench_spscq
Run on (20 X 2918.4 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x10)
  L1 Instruction 32 KiB (x10)
  L2 Unified 1280 KiB (x10)
  L3 Unified 24576 KiB (x1)
Load Average: 0.27, 0.22, 0.19
***WARNING*** Library was built as DEBUG. Timings may be affected.
-------------------------------------------------------------------------
Benchmark               Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------
BM_PushLatency       3.22 ns         3.22 ns    165144506 items_per_second=310.145M/s
BM_PushPop           1.67 ns         1.67 ns    425541228 items_per_second=599.318M/s
BM_Throughput        3.27 ns         3.29 ns    190241792 items_per_second=304.148M/s
BM_PushNaive         5.89 ns         5.92 ns    120521963 items_per_second=168.861M/s
```