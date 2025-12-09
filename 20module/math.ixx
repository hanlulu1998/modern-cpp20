export module math;

export int add(int, int);

// 模块天然保证ODR
export double pi() {
    return 3.1415926;
}
