# CMake 笔记

## Makefile vs CMake

**本质关系：**

```
CMakeLists.txt  ← 你写的
      ↓
CMake 生成
      ↓
Makefile / Ninja / MSBuild  ← 最终执行
```

CMake 本身**不是构建器**，是构建系统的**生成器**。

| 场景              | 推荐                                  |
| ----------------- | ------------------------------------- |
| 大型 C/C++ 项目   | CMake（跨平台、依赖管理、生态标准）   |
| 小型项目 / 单文件 | Makefile（一行 `make` 比跑 CMake 快） |
| 嵌入式/裸机       | Makefile（无复杂依赖链）              |

## Makefile 依赖解析

Make 根据依赖图**自动排序**，不需要手动排编译顺序：

```makefile
program: a.o b.o c.o        # 先编译 .o，再链接
    gcc -o program a.o b.o c.o

a.o: a.cpp a.h              # .o 依赖 .cpp 和 .h
    gcc -c a.cpp
```

- `program` → 发现需要 `a.o` → 发现需要 `a.cpp` → 编译 `a.cpp`
- 三个 `.o` 齐了 → 链接成 `program`
- **只编译有变化的文件**

CMake 解决了手写依赖图的麻烦，自动扫描 + 跨平台链接。

## 快速参考

### 基础

```cmake
cmake_minimum_required(VERSION 3.14)
project(my_code LANGUAGES CXX)   # LANGUAGES 避免误编译 C 文件

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
```

### 输出目录

```cmake
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
```

### 源文件与目标

```cmake
add_executable(LOGO LOGO.cpp)
add_executable(atomic_signal atomic_signal.cpp)
```

**不要用 `aux_source_directory`**（不追踪新文件、混入非源文件）。
用 `file(GLOB ...)` 勉强可用，但显式列出最安全。

### 编译选项

```cmake
# 全局（所有目标）
add_compile_options(-Wall -Wextra)

# 单个目标
target_compile_options(LOGO PRIVATE -O2)
target_compile_options(atomic_signal PRIVATE -O2)
```

### 链接库

```cmake
target_link_libraries(atomic_signal PRIVATE pthread)
```

| 关键字      | 含义                    |
| ----------- | ----------------------- |
| `PRIVATE`   | 只链接到当前目标        |
| `PUBLIC`    | 当前目标 + 传递给依赖方 |
| `INTERFACE` | 只传给依赖方            |

### 清理

```bash
# 始终用独立目录构建（out-of-source）
mkdir build && cd build
cmake ..
cmake --build .

# 清理
cd .. && rm -rf build/
```
