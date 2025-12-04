#include <initializer_list>
#include <iostream>
#include <map>
#include <string>
#include <vector>
class T {
    std::vector<int> data_;

  public:
    std::vector<int> &items() {
        return data_;
    }

    T(const std::initializer_list<int> &list) : data_(list) {
    }
};

T foo() {

    return T{1, 2, 3, 4, 5};
}

// 5. 实现自己的rang for
// 1）这个类型需要有begin和end函数，可以是类成员函数也可以独立函数
// 2）begin和end需要返回迭代器对象
// 3）迭代器对象必须支持*、!=、++(前缀)这三个运算符，也就是是个包装指针
class IntIter {
    int *p_;

  public:
    IntIter(int *p) : p_(p) {
    }

    bool operator!=(const IntIter &other) {
        return p_ != other.p_;
    }

    const IntIter &operator++() {
        p_++;
        return *this;
    }

    int operator*() const {
        return *p_;
    }
};

template <unsigned int SIZE>
class IntArray {
    int data_[SIZE]{0};

  public:
    IntArray(std::initializer_list<int> list) {
        int *cur = data_;
        for (auto e : list) {
            *cur = e;
            cur++;
        }
    }

    IntIter begin() {
        return IntIter(data_);
    };

    IntIter end() {
        return IntIter(data_ + SIZE);
    }
};

int main(void) {

    std::map<int, std::string> index_map{{1, "hello"}, {2, "world"}, {3, "!!!"}};
    int int_array[]{1, 2, 3, 4, 5, 6};

    // 1. 支持引用
    for (const auto &e : index_map) {
        std::cout << e.first << " : " << e.second << "\n";
    }
    // 2. 支持值复制
    for (auto e : int_array) {
        std::cout << e << ", ";
    }
    std::cout << "\n";

    // 3. 现在c++对范围for进行了改进，不要求begin和end返回相同类型

    // 4. range for的陷阱
    // range for的开始伪代码如下：auto && __range = range_expression
    // 这里的auto&&万能引用，如果是个右值没有问题，右值引用会拓展周期
    // 如果是泛左值，那么就是左值引用，会导致一些问题

    for (auto &x : foo().items()) {
        std::cout << x << ", ";
    }
    std::cout << "\n";

    for (const auto &x : foo().items()) {
        std::cout << x << ", ";
    }
    std::cout << "\n";

    // 现在rang for开始支持初始化
    for (T thing = foo(); auto &x : thing.items()) {
        std::cout << x << ", ";
    }
    std::cout << "\n";

    // 5. 实现自己的rang for
    IntArray<10> array{1, 2, 3, 4, 5, 6};
    for (auto e : array) {
        std::cout << e << ", ";
    }
    std::cout << "\n";

    return 0;
}
