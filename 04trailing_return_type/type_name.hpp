#pragma once
#include <string>
#include <type_traits>
#include <typeinfo>
template <typename T>
struct type_name {
    static std::string get() {
        if constexpr (std::is_const_v<T>)
            return "const " + type_name<std::remove_const_t<T>>::get();
        else if constexpr (std::is_volatile_v<T>)
            return "volatile " + type_name<std::remove_volatile_t<T>>::get();
        else if constexpr (std::is_lvalue_reference_v<T>)
            return type_name<std::remove_reference_t<T>>::get() + "&";
        else if constexpr (std::is_rvalue_reference_v<T>)
            return type_name<std::remove_reference_t<T>>::get() + "&&";
        else if constexpr (std::is_pointer_v<T>)
            return type_name<std::remove_pointer_t<T>>::get() + "*";
        else if constexpr (std::is_array_v<T>) {
            return type_name<std::remove_extent_t<T>>::get() + "[" +
                   std::to_string(std::extent_v<T>) + "]";
        } else
            return typeid(T).name(); // 基础类型由编译器提供
    }
};

template <typename T>
std::string type_name_v = type_name<T>::get();
