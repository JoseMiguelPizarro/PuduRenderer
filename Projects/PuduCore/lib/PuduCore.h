#pragma once
#include <memory>
#include <filesystem>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

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
    using Size = std::size_t;
    using vec2 = glm::vec2;
    using vec3 = glm::vec3;
    using vec4 = glm::vec4;
    using mat4 = glm::mat4;
    using mat3 = glm::mat3;
    using mat2 = glm::mat2;

    namespace fs = std::filesystem;
}
