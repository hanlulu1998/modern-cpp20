# cmake版本至少在3.28+
cmake_minimum_required(VERSION 3.28)
project(math_module)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(math_module )
target_sources(math_module PRIVATE
    main.cpp
    a.cpp
    math_impl.cpp
)
# 接口文件要采用这种方式单独声明
target_sources(math_module PRIVATE
    FILE_SET CXX_MODULES
    FILES
    math.ixx
)

