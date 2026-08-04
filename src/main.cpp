
#include <iostream>
#include <thread>
#include <atomic>
#include "ring_buffer.hpp"
#include "wait_strategy.hpp"

int main() {
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

    std::this_thread::sleep_for(std::chrono::seconds(5));
    prod_running.store(false, std::memory_order_relaxed);
    con_running.store(false, std::memory_order_relaxed);
    producer.join();
    consumer.join();
    std::cout << "Pushed: " << push_count.load() << "\n";
    std::cout << "Popped: " << pop_count.load() << "\n";
    std::cout << "Throughput: " << pop_count.load() / 5 << " msgs/sec\n";
    return 0;
}

/*
build and test
mkdir -p build && cd build
cmake ..
cmake .. -DMAKE_BUILD_TYPE=Release
cmake --build . --clean-first

*/
