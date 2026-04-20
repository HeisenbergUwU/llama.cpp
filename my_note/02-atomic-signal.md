# std::atomic 原子操作学习笔记

## 1. 基本用法

### 声明原子变量
```cpp
#include <atomic>

static std::atomic<bool> g_is_interrupted(false);  // 直接初始化 ✅
// static std::atomic<bool> g_is_interrupted{false};  // 统一初始化 ✅
```

**注意**：`std::atomic` **不支持拷贝初始化**，不能用 `=`：
```cpp
static std::atomic<bool> g_is_interrupted = false;  // ❌ 编译错误！
```

- `std::atomic<T>` 模板，T 可以是 bool, int, pointer 等
- 保证对该变量的操作是原子的，不会产生命竞态条件
- `= false` 是拷贝初始化，会调用拷贝构造，但 `atomic` 的拷贝构造被删除了
- 改用 `()` 或 `{}` 直接初始化即可

### 读取（load）
```cpp
bool value = g_is_interrupted.load();
```

### 写入（store）
```cpp
g_is_interrupted.store(true);
```

## 2. 在信号处理中的应用

### 场景：处理 Ctrl+C 中断

```cpp
static std::atomic<bool> g_stop = false;

static void signal_handler(int) {
    if (g_stop.load()) {
        // 第二次 Ctrl+C - 立即退出
        fprintf(stdout, "\033[0m\n");  // 重置终端颜色
        fflush(stdout);
        std::exit(130);
    }
    // 第一次 Ctrl+C - 设置停止标志
    g_stop.store(true);
}

int main() {
    signal(SIGINT, signal_handler);
    
    // 模拟任务
    for (int i = 0; i < 100; i++) {
        if (g_stop.load()) {
            printf("Task stopped gracefully\n");
            return 0;
        }
        printf("Processing %d\n", i);
        sleep(1);
    }
    return 0;
}
```

### 为什么用 atomic 而不是普通 bool？

1. **信号处理函数中的线程安全问题**：
   - 信号可以在任何时刻被触发，即使在读取普通变量
   - 普通 bool 的读写不是原子的，可能遇到竞态条件

2. **避免死锁**：
   - 信号处理函数中不能使用 mutex（因为信号可能在持有锁时触发）
   - atomic 操作不需要锁，因此是信号安全的

3. **内存可见性**：
   - atomic 保证写操作对其他线程立即可见
   - 普通变量可能被编译器优化导致其他线程看不到更新

## 3. C++11 原始字符串 R"()"

### 语法
```cpp
const char* str = R"(
    第一行
    第二行
    包含 "引号" 和 \ 反斜杠
)";
```

### 特点
- 无需转义双引号和反斜杠
- 保留换行符和空格
- 适合存储多行文本、HTML、JSON 等

### llama.cpp 中的应用
```cpp
const char * LLAMA_ASCII_LOGO = R"(
▄▄ ▄▄
██ ██
...
)";
```

## 4. 编译测试

### 编译命令
```bash
clang++ -std=c++17 atomic_signal.cpp -o atomic_signal
```

### 测试方法
1. 运行程序：`./atomic_signal`
2. 第一次按 Ctrl+C：看到 "Interrupt received!"
3. 第二次按 Ctrl+C：程序立即退出

## 5. 关键要点

1. `std::atomic` 用于多线程/信号处理中的共享变量
2. `.load()` 和 `.store()` 是原子读写操作
3. 信号处理函数中避免使用 mutex，应使用 atomic
4. 退出码 130 = 128 + 2（SIGINT 信号编号）
5. ANSI 转义序列 `\033[0m` 用于重置终端颜色
