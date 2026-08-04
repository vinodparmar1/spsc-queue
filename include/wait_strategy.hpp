
#pragma once

#include <thread>
#include <chrono>

namespace spsc {

struct SpinWait {
    void wait() {
        // do nothing - instant retry
    }
};

struct YieldWait{
    void wait() {
        std::this_thread::yield();
    }
};

struct SleepWait {
    void wait() {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
};

struct BlockWait {
    void wait() {

    }
};
}
