#include <algorithm>
#include <any>
#include <chrono>
#include <ctime>
#include <execution>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <random>
#include <regex>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <variant>
#include <vector>

struct Point {
    int x;
    int y;
};

// 使用多继承，实现多个匹配的仿函数
struct Overloaded {
    template <class T>
    void operator()(T &&) const = delete; // 可选
};

template <class... Ts>
struct Overload : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
Overload(Ts...) -> Overload<Ts...>;

// 自定义format
template <>
struct std::formatter<Point> {
    constexpr auto parse(auto &ctx) {
        return ctx.begin();
    }

    auto format(const Point &p, auto &ctx) const {
        return std::format_to(ctx.out(), "({}, {})", p.x, p.y);
    }
};

int main(void) {
    // 1. 时间日期库
    // 1）支持字面量
    using namespace std::chrono_literals;
    // duration时间段，time_point时间点，clock时钟
    std::chrono::seconds s(5);
    auto t = s + 3h + 15min - 1s;
    std::cout << t << ": " << std::chrono::duration_cast<std::chrono::minutes>(t) << "\n";

    // 2）system_clock系统时间，但用户可以调，steady_clock单调时间，多用于测时间，high_resolution_clock最高精度时间
    auto start = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(2s);
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    std::cout << std::chrono::duration_cast<std::chrono::seconds>(diff) << "\n";

    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    // 传统c方式
    std::tm local_tm = *std::localtime(&now_time);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &local_tm);
    std::cout << buf << "\n";
    // c++方式
    // 使用c++20的format库
    std::cout << std::format("{:%F %T}", now) << "\n";

    // 3) 日期
    // 字面量和/符号
    auto d = 2025y / 11 / 20;
    auto d2 = d + std::chrono::months{3};
    std::cout << d2 << "\n";

    // 4）时区转换，支持力度不同
    // auto local = zoned_time{current_zone(), now};
    // std::cout << std::format("{:%Y-%m-%d %H:%M:%S}\n", local);

    // 2. foramt库：类似于python的现代格式化库，替代以前的stringstream和c的sprintf
    Point p{1, 2};
    std::cout << std::format("Point is {}", p) << "\n";
    // 一些新的字符串功能
    std::cout << std::stoi("120") << "\n";
    std::cout << std::stoll("100") << "\n";
    std::cout << std::to_string(11.2) << "\n";

    // 3. 统一的随机库
    // 1）随机数种子，可以使用时间，或者使用以下的
    std::random_device rd{};

    // 2) 随机数引擎mt19937和mt19937_64
    // 多次使用引擎应该初始化一次，防止伪随机
    std::mt19937 rng{rd()};

    // 3) 分布函数
    // 均匀分布
    std::uniform_int_distribution<int> dist(1, 100);
    std::cout << dist(rng) << "\n";
    // 正态分布
    std::normal_distribution<double> norm(0.0, 1.0);
    std::cout << norm(rng) << "\n";

    // 4. 正则表达库，由于性能很差，在简单环境可用
    // 复杂表达推荐RE2
    std::regex re(R"(\d{4}-\d{2}-\d{2})");
    if (std::regex_match("2024-10-01", re)) {
        std::cout << "match ok\n";
    }

    // 5. 文件操作库
    namespace fs = std::filesystem;
    for (auto &entry : fs::directory_iterator("./")) {
        std::cout << entry.path() << "\n";
    }

    if (!fs::exists("./fs")) {
        fs::create_directories("./fs");
    }

    std::ofstream ofs("./fs/test.txt");
    if (ofs.is_open()) {
        ofs << "Hello world\n";
    }
    fs::copy("./fs/test.txt", "./fs/tmp.txt", fs::copy_options::overwrite_existing);
    fs::remove("./fs/test.txt");

    // 获取时间戳
    // 这里的支持有问题，file_clock可以使用clock_cast转换
    // 转换成system_clock获得正确的时间戳
    auto ts = fs::last_write_time("./fs/tmp.txt");
    std::cout << ts << "\n";

    // 6. 并行算法
    std::vector<int> vec{1, 5, 6, -10, 11, 0, 7, 8, 1, 3, 9, 0, -3};
    // 编译器后端要支持
    // std::sort(std::execution::par, vec.begin(), vec.end());

    // 7. 几个现代类型
    // 1）std::optional
    auto i = std::optional{1};
    if (i) {
        std::cout << i.value() << "\n";
    }
    i = std::nullopt;
    if (!i.has_value()) {
        std::cout << i.value_or(-1) << "\n";
    }

    // 2）任意擦出类型
    std::any a = 10;
    // 使用字符串字面量后缀
    using namespace std::string_literals;
    a = "hello"s;

    try {
        std::cout << std::any_cast<std::string>(a) << "\n";
    } catch (const std::bad_any_cast &) {
        std::cout << "Type is not matched\n";
    }

    // 3) 安全联合体，更好的性能
    std::variant<int, std::string> v;
    v = 42;
    std::cout << std::get<int>(v) << "\n";
    v = "hello"s;
    std::cout << std::get<std::string>(v) << "\n";
    // 使用std::visit访问，可以直接使用lambda pack，std::overload函数不稳定
    std::visit(Overload{
                   [](int x) { std::cout << "int :" << x << "\n"; },
                   [](const std::string &s) { std::cout << "string: " << s << "\n"; },
               },
               v);

    // 4) 零拷贝字符串视图
    // 只拥有引用，不做任何拷贝，要保证生命周期
    auto println = [](std::string_view sv) {
        std::cout << sv << "\n";
    };
    // 可以接受任何形式的字符串
    println("hello"s);
    println("hello");
    // 可以切片
    std::string_view sv = "hello";
    std::cout << sv << "\n";
    auto sub_sv = sv.substr(1, 3);
    std::cout << sub_sv << "\n";

    // 5) 非拥有的连续内存视图
    // 持有引用，不复制数据，只要是连续内存都可以
    // 相当于T *ptr + size_t n的组合
    auto print2 = [](std::span<const char> s) {
        for (const auto c : s) {
            std::cout << c;
        }
        std::cout << "\n";
    };

    char arr[]{"hello"};

    // 只要是连续内存即可
    print2("hello"s);
    print2(arr);
    print2("hello");
    return 0;
}
