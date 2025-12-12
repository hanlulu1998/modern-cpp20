#include <cstddef>
#include <expected>
#include <format>
#include <mdspan>
#include <print>
#include <string>

std::expected<int, std::string> divide(int a, int b) {
    if (b == 0)
        return std::unexpected<std::string>("can not divided 0");
    return a / b;
}

int main(void) {
    // 1. 现代类型的打印
    // 是format的打印包装，自定义类型需要实现format
    std::println("int:{}, string:{}", 1, "Hello C++23");

    // 2. 多维度span
    int array[]{1, 2, 3, 4, 5, 6};
    std::mdspan<int, std::extents<size_t, 2, 3>> mat(array);
    // std::println("{}", mat(1, 2));

    // 3. 新的异常处理，Rust风格Result<T,E>
    auto r = divide(10, 2);
    std::println("{}", r.value());

    auto e = divide(10, 0);
    std::println("{}", e.error());

    return 0;
}
