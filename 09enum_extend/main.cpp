#include <cstdio>
#include <iostream>
enum class HighSchool {
    student,
    teacher,
    principal
};

enum class University {
    student,
    teacher,
    principal
};

enum class Color1 {};
enum class Color2 : char {};
enum class Color3 : short {};

enum class Color {
    Red,
    Green,
    Blue
};

// 2. 可以使用using开放枚举变量
const char *ColorToString(Color c) {
    switch (c) {
        using enum Color;
        using Color::Red;
    case Red:
        return "Red";
    case Green:
        return "Green";
    case Blue:
        return "Blue";
    default:
        return "none";
    }
}

int main(void) {
    // 1. 强枚举类型
    // 1）不同于普通枚举，强枚举需要自己的作用域
    auto x = HighSchool::student;

    // 2)无法隐式转换为int，虽然默认类型为int
    // int y = HighSchool::teacher;
    int y = static_cast<int>(HighSchool::teacher);

    // 3) 由于没有隐式转换，所以不同枚举不可比较
    // HighSchool::student < University::student;
    bool b = HighSchool::student < HighSchool::teacher;
    printf("%d\n", b);

    // 4）支持初始化列表
    HighSchool h{1};
    // 本质上还是比较底层值
    bool d = HighSchool::teacher == h;
    printf("%d\n", d);

    Color c{1};
    Color t{1};
    bool m = c == t;
    printf("%d\n", m);

    printf("%s\n", ColorToString(c));

    Color1 c1{1};
    Color2 c2{'a'};
    Color3 c3{1};
    printf("%ld,%ld,%ld,%ld\n", sizeof(c), sizeof(c1), sizeof(c2), sizeof(c3));

    return 0;
}
