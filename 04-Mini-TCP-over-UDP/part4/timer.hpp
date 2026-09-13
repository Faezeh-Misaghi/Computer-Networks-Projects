#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>

using namespace std::chrono;

class Timer {
private:
    time_point<high_resolution_clock> start_time;
    double timeout_ms;
    bool running;

public:
    Timer(double timeout_ms = 1000.0) {
        this->timeout_ms = timeout_ms;
        this->running = false;
    }
    void start() {
        start_time = high_resolution_clock::now();
        running = true;
    }
    void stop() {
        running = false;
    }

    bool is_timeout() {
        if (!running) return false;
        auto current_time = high_resolution_clock::now();
        duration<double, std::milli> elapsed = current_time - start_time;
        return elapsed.count() >= timeout_ms;
    }
};

#endif