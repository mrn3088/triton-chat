#include "../thirdparty/json.hpp"
using nlohmann::json;

#include <iostream>
#include <map>
#include <vector>
#include <string>

void func1() {
    json js;
    js["msg_type"] = 2;
    js["from"] = "first";
    js["to"] = "second";
    js["msg"] = "hello world";
    std::cout << js << std::endl;
    std::cout << js.dump() << std::endl;
}

void func2() {
    json js;

    js["id"] = {1, 2, 3, 4, 5};
    js["name"] = {"zhangsan", "lisi", "wangwu"};

    js["msg"]["id"] = 1000;
    js["msg"]["name"] = "zhangsan";
    std::cout << js.dump() << std::endl;
}

void func3() {
    json js;
    std::vector<int> li = {1, 2, 3, 4, 5};
    js["list"] = li;

    std::unordered_map<int, std::string> mp;
    mp.emplace(1, "val1");
    mp.emplace(2, "val2");
    mp.emplace(3, "val3");
    js["map"] = mp;

    std::cout << js.dump() << std::endl;
}

int main() {
    func1();
    func2();
    func3();
}