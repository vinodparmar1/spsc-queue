
#pragma once

#include <array>
#include <atomic>
#include "wait_strategy.hpp"

namespace spsc {

template<typename T, size_t Capacity>
class SPSCQueue {
static_assert((Capacity & (Capacity -1)) == 0, "Capacity should be power of 2");

public:
    bool push(const T& item) {
        auto tail = tail_.load(std::memory_order_relaxed);
        if(tail == cached_head_ + Capacity) {
            // looks buffer overflow
            cached_head_ = head_.load(std::memory_order_acquire);
            if(tail == cached_head_ + Capacity) {
                // confirmed overflow
                return false;
            }
        }
        rbuffer_[tail & mask_] = item;
        tail_.store(tail + 1, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        auto head = head_.load(std::memory_order_relaxed);
        if(head == cached_tail_) {
            // Looks like buffer underflow
            cached_tail_ = tail_.load(std::memory_order_acquire);
            if(head == cached_tail_) {
                // confirmed underflow
                return false;
            }
        }
        item = rbuffer_[head & mask_];
        head_.store(head+1, std::memory_order_release);
        return true;
    }

    SPSCQueue() = default;
    SPSCQueue(const SPSCQueue&) = delete;
    SPSCQueue& operator=(const SPSCQueue&) = delete;
    SPSCQueue(SPSCQueue&&) = delete;
    SPSCQueue& operator=(SPSCQueue&&) = delete;

private:
    alignas(64) std::atomic<uint64_t> head_{0};
    uint64_t cached_tail_ = 0;
    alignas(64) std::atomic<uint64_t> tail_{0};
    uint64_t cached_head_ = 0;
    static constexpr size_t mask_ = Capacity - 1;
    alignas(64) std::array<T, Capacity> rbuffer_;
};

}
