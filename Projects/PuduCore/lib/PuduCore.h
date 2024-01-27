#pragma once
#include <memory>
#include <filesystem>

template<typename T>
using SPtr = std::shared_ptr<T>;
using Handle = uint32_t;


