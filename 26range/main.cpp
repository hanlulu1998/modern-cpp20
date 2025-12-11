#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
// ranges简单理解为c++的流式操作，经典的惰性运算
// range只要有begin和end就是一个range
// range -> pipe -> views... -> algorithm

int main(void) {
    std::vector vec{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    // 1. 基本的views
    // 1) 过滤 + 映射
    auto r1 =
        vec | std::views::filter([](int x) { return x % 2 == 0; }) | std::views::transform([](int x) { return x * x; });

    for (int x : r1) {
        std::cout << x << ' ';
    }
    std::cout << "\n";

    // 2) take+drop
    for (int x : vec | std::views::drop(2) | std::views::take(3))
        std::cout << x << ' ';
    std::cout << "\n";

    // 3) 字符串split
    using namespace std::string_literals;
    for (auto part : std::views::split("abc:efg:hij"s, ':')) {
        std::cout << std::string_view(part) << ' ';
    }
    std::cout << "\n";

    // 4) zip
    std::vector a{1, 2, 3};
    std::vector b{10, 20, 30};

    for (auto [x, y] : std::views::zip(a, b))
        std::cout << x + y << ' ';
    std::cout << "\n";

    // 5) join(flatten)
    std::vector<std::vector<int>> v{{1, 2}, {3, 4}};

    for (int x : v | std::views::join)
        std::cout << x << ' ';
    std::cout << "\n";

    // 6) keys和values
    std::map<std::string, int> mp{{"a", 1}, {"b", 2}};

    for (auto k : std::views::keys(mp))
        std::cout << k << "\n";
    for (auto v : std::views::values(mp))
        std::cout << v << "\n";

    // 2. ranges算法
    auto sum = std::ranges::fold_left(r1, 0, std::plus{});
    std::cout << sum << "\n";
    std::cout << std::ranges::contains(vec, 2) << "\n";
    std::cout << std::ranges::contains(r1, 4) << "\n";

    // 混合使用
    auto result =
        std::ranges::fold_left(
            vec | std::views::filter([](int x) { return x % 2 == 0; }) | std::views::transform([](int x) { return x * x; }) | std::views::take(10),
            0,
            std::plus{});

    std::cout << result << "\n";

    return 0;
}
