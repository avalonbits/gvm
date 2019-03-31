#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>

void thread1(bool* sem, int* value, std::mutex* m, std::condition_variable* var) {
    bool stop = false;
    while (!stop) {
        {
            // Read the input.
            std::unique_lock<std::mutex> lk(*m);
            var->wait(lk, [sem]() { return *sem; });
            std::cerr << "Thread 1: " << *value << std::endl;
        }
        {
            // Write back.
            std::lock_guard<std::mutex> lk(*m);
            ++(*value);
            *sem = false;
        }
        if (*value >= 100) {
            stop = true;
            std::cerr << "Thread 1: bye!\n";
        }
        var->notify_all();
    }
}

void thread2(bool* sem, int* value, std::mutex* m, std::condition_variable* var) {
    bool stop = false;
    while (!stop) {
        {
            // Write the input
            std::lock_guard<std::mutex> lk(*m);
            ++(*value);
            *sem = true;
        }
        var->notify_all();

        {
            // Read the input.
            std::unique_lock<std::mutex> lk(*m);
            var->wait(lk, [sem]() { return !(*sem); });
            std::cerr << "Thread 2: " << *value << std::endl;
            if (*value >= 100) stop = true;
        }
    }
    std::cerr << "Thread 2: bye!\n";
}


int main(void) {
    bool sem = false;
    int value = 0;
    std::mutex m;
    std::condition_variable var;

    std::thread t1(thread1, &sem, &value, &m, &var);
    std::thread t2(thread2, &sem, &value, &m, &var);

    t1.join();
    t2.join();
    return 0;
}
