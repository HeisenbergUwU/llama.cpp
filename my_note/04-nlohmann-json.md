# nlohmann/json 原理

## 概述

nlohmann/json 是一个**单头文件 C++ JSON 库**，所有代码包含在 `json.hpp` 中 (2.5 万行)。

## 核心原理

### 1. 联合体存储 (Variant Pattern)

用 `union` + 类型标签实现类型安全的变体存储：

```cpp
union {
    number_integer_t m_data_integer;
    number_unsigned_t m_data_unsigned;
    number_float_t m_data_float;
    binary_t* m_data_binary;
    string_t* m_data_string;
    array_t* m_data_array;
    object_t* m_data_object;
};
value_t m_type;  // 类型标签
```

支持的 JSON 类型：`null` / `object` / `array` / `string` / `number` / `boolean`

### 2. 类型标签 + 访问控制

- 所有操作先检查 `m_type`，类型不匹配抛异常
- 访问函数 (`at`, `operator[]`) 自动转换或创建类型

### 3. SFINAE + 模板元编程

自动序列化/反序列化自定义类型：

```cpp
// 检测 to_json/from_json 函数
template<typename T>
static auto to_json(T& j, const T& val) -> decltype(j = val);
```

利用 ADL (Argument-Dependent Lookup) 支持：
- 标准类型自动转换
- 自定义结构体需定义 `to_json`/`from_json`

### 4. 代理模式 (链式调用)

```cpp
json j;
j["name"]["first"] = "John";  // 自动创建中间对象
```

`operator[]` 返回引用代理，缺失的 key 自动创建 `null` 值。

### 5. 零拷贝优化

- 字符串/数组/对象用指针存储
- 移动语义减少拷贝

### 6. 解析器实现

手写递归下降解析器 (LL(1))，支持：
- 流式解析 (iterator)
- SAX 风格事件驱动解析
- 二进制格式 (CBOR, MessagePack, BSON)

## 单头文件特点

| 传统库 | 单头文件库 |
|--------|-----------|
| 多个 `.hpp` + `.cpp` | 只有一个 `json.hpp` |
| 需要编译/链接 | 直接 `#include` |
| 需要链接 `.a`/`.so` | 无需链接 |

## 使用示例

```cpp
#include "vendor/nlohmann/json.hpp"
using json = nlohmann::json;

// 序列化
json j = {{"name", "test"}, {"value", 42}};
std::string str = j.dump();

// 反序列化
json j2 = json::parse(str);
std::string name = j2["name"];
```

## 优缺点

**优点：**
- 零依赖，复制即用
- 没有编译/链接步骤
- 版本管理简单

**缺点：**
- 文件巨大 (编译慢)
- 无法单独更新模块

## 相关文件

- `vendor/nlohmann/json.hpp` - 主文件
- `vendor/nlohmann/json_fwd.hpp` - 前向声明
