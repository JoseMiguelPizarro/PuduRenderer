#pragma once
#include <type_traits>

template <typename T, typename U>
concept Derived = std::is_base_of_v<U, T>;
