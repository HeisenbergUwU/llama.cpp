#include <signal.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

// 全局原子变量，用于线程间通信
static std::atomic<bool> g_stop(false);

// 信号处理函数 - 处理 Ctrl+C
static void signal_handler(int signal) {
    if (g_stop.load()) {
        // 第二次 Ctrl+C - 立即退出
        std::cerr << "\nForce exit!\n";
        std::exit(130);
    }
    // 第一次 Ctrl+C - 设置停止标志
    g_stop.store(true);
    std::cerr << "\nInterrupt received! Press Ctrl+C again to force exit.\n";
}

// 模拟长时间运行的任务
void long_running_task() {
    for (int i = 0; i < 100; i++) {
        if (g_stop.load()) {
            std::cout << "\nTask gracefully stopped at iteration " << i << "\n";
            return;
        }
        std::cout << "Processing: " << i << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    // 注册信号处理器
    signal(SIGINT, signal_handler);

    std::cout << "Starting long-running task...\n";
    std::cout << "Press Ctrl+C to interrupt (once for graceful stop, twice for force exit)\n";

    long_running_task();

    std::cout << "Task finished normally.\n";
    return 0;
}
