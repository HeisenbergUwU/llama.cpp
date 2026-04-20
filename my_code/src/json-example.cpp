#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

struct User {
    std::string name;
    int age;
    std::string email;
};

void to_json(nlohmann::json& j, const User& u) {
    j = nlohmann::json{{"name", u.name}, {"age", u.age}, {"email", u.email}};
}

void from_json(const nlohmann::json& j, User& u) {
    j.at("name").get_to(u.name);
    j.at("age").get_to(u.age);
    j.at("email").get_to(u.email);
}

int main() {
    std::cout << "=== 1. 创建 JSON ===" << std::endl;
    json j;
    j["name"] = "张三";
    j["age"] = 25;
    j["active"] = true;
    j["score"] = 95.5;
    std::cout << j.dump() << std::endl;

    std::cout << "\n=== 2. 嵌套结构 ===" << std::endl;
    json nested;
    nested["user"] = {
        {"name", "李四"},
        {"hobbies", {"reading", "coding"}}
    };
    nested["settings"] = {
        {"theme", "dark"},
        {"language", "zh"}
    };
    std::cout << nested.dump(2) << std::endl;

    std::cout << "\n=== 3. 数组 ===" << std::endl;
    json arr = json::array({1, 2, 3, "four"});
    std::cout << arr.dump() << std::endl;
    for (size_t i = 0; i < arr.size(); i++) {
        std::cout << "  [" << i << "] = " << arr[i] << std::endl;
    }

    std::cout << "\n=== 4. 访问与解析 ===" << std::endl;
    json data = R"({
        "id": 1,
        "items": [10, 20, 30],
        "metadata": {"created": "2024-01-01"}
    })"_json;
    std::cout << "id = " << data["id"] << std::endl;
    std::cout << "items[1] = " << data["items"][1] << std::endl;
    std::cout << "metadata.created = " << data["metadata"]["created"] << std::endl;
    std::cout << "默认值: tags = " << data.value<std::string>("tags", "none") << std::endl;

    std::cout << "\n=== 5. 迭代遍历 ===" << std::endl;
    for (auto& [key, value] : data.items()) {
        std::cout << "  " << key << " => " << value << std::endl;
    }

    std::cout << "\n=== 6. 类型判断 ===" << std::endl;
    json types;
    types["a"] = 42;
    types["b"] = 3.14;
    types["c"] = "hello";
    types["d"] = nullptr;
    types["e"] = true;
    types["f"] = json::array();
    types["g"] = json::object();

    for (auto& [key, value] : types.items()) {
        std::cout << "  " << key << ": type=" << value.type_name();
        std::cout << " is_number=" << value.is_number();
        std::cout << " is_string=" << value.is_string();
        std::cout << " is_object=" << value.is_object() << std::endl;
    }

    std::cout << "\n=== 7. 自定义结构体序列化 ===" << std::endl;
    User u = {"王五", 30, "wang@example.com"};
    json j_user = u;
    std::cout << "to_json: " << j_user.dump() << std::endl;

    User u2 = j_user.get<User>();
    std::cout << "from_json: name=" << u2.name << " age=" << u2.age << std::endl;

    std::cout << "\n=== 8. ordered_json (保持 key 顺序) ===" << std::endl;
    ordered_json oj;
    oj["z"] = 1;
    oj["a"] = 2;
    oj["m"] = 3;
    std::cout << "ordered: " << oj.dump() << std::endl;
    json oj_normal = oj;
    std::cout << "normal:  " << oj_normal.dump() << std::endl;

    std::cout << "\n=== 9. 序列化到 stringstream ===" << std::endl;
    json stream_json = {{"a", 1}, {"b", 2}};
    std::stringstream ss;
    ss << stream_json;
    std::cout << "stream: " << ss.str() << std::endl;

    json parsed;
    std::stringstream ss2(ss.str());
    ss2 >> parsed;
    std::cout << "parsed: " << parsed.dump() << std::endl;

    std::cout << "\n=== 10. 异常处理 ===" << std::endl;
    try {
        json bad = json::parse("{invalid}");
    } catch (json::parse_error& e) {
        std::cout << "parse error: " << e.what() << std::endl;
    }

    try {
        json missing = R"({"a": 1})"_json;
        missing["x"]["y"]["z"] = 100;
    } catch (json::type_error& e) {
        std::cout << "type error: " << e.what() << std::endl;
    }

    std::cout << "\n=== 11. JSON Pointer ===" << std::endl;
    json pointer_data = R"({
        "foo": {"bar": [1, 2, 3], "baz": "qux"}
    })"_json;
    std::cout << "pointer_data[/foo/bar/0] = " << pointer_data["/foo/bar/0"].get<int>() << std::endl;
    std::cout << "pointer_data[/foo/baz] = " << pointer_data["/foo/baz"] << std::endl;

    std::cout << "\n=== 12. 从 C++ 容器转换 ===" << std::endl;
    std::vector<int> vec = {10, 20, 30};
    json j_vec = vec;
    std::cout << "vector: " << j_vec.dump() << std::endl;

    std::map<std::string, int> map = {{"a", 1}, {"b", 2}};
    json j_map = map;
    std::cout << "map: " << j_map.dump() << std::endl;

    return 0;
}
