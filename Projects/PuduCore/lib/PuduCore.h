#pragma once
#include <memory>
#include <filesystem>
#include "PuduConstants.h"

namespace Pudu
{
    template <typename T>
    using SPtr = std::shared_ptr<T>;
    using Handle = uint32_t;

    using u8 = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;
    using i8 = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;
    using f32 = float;
    using f64 = double;
    using size = std::size_t;
    namespace fs = std::filesystem;
}
