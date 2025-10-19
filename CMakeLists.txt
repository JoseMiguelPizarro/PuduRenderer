cmake_minimum_required(VERSION 3.21)

project(PuduRenderer LANGUAGES CXX)
include(FetchContent)

# ----------------------------------------------------
# Global Settings
# ----------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_GENERATOR_PLATFORM x64)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs")

set(CMAKE_DISABLE_FIND_PACKAGE_SLANG ON)
set(CMAKE_DISABLE_FIND_PACKAGE_GLSLANG ON)

# ----------------------------------------------------
# Vulkan SDK
# ----------------------------------------------------
set(VULKAN_SDK $ENV{VULKAN_SDK})

# ----------------------------------------------------
# KTX
# ----------------------------------------------------
find_package(KTX REQUIRED)

# ----------------------------------------------------
# Slang Setup
# ----------------------------------------------------
set(SLANG_VERSION "2025.16.1")

if (WIN32)
    set(SLANG_OS "windows-x86_64")
elseif (APPLE)
    set(SLANG_OS "macos-aarch64")
elseif (UNIX)
    set(SLANG_OS "linux-x86_64")
endif()

set(DEPENDENCIES_PATH "_deps")
message(STATUS "Fetching Slang v${SLANG_VERSION} for ${SLANG_OS}")

FetchContent_Populate(
        slang_zip
        URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-${SLANG_OS}.zip
        SOURCE_DIR "${DEPENDENCIES_PATH}/Slang/src"
        BINARY_DIR "${DEPENDENCIES_PATH}/Slang/bin"
        SUBBUILD_DIR "${DEPENDENCIES_PATH}/Slang/subBuild"
        QUIET

)

set(SLANG_INCLUDE_DIR "${slang_zip_SOURCE_DIR}/include")
set(SLANG_LIB_DIR "${slang_zip_SOURCE_DIR}/lib")
set(SLANG_BIN_DIR "${slang_zip_SOURCE_DIR}/bin")

if (WIN32)
    set(SLANG_LIB_FILE "${SLANG_LIB_DIR}/slang.lib")
elseif (APPLE)
    set(SLANG_LIB_FILE "${SLANG_LIB_DIR}/libslang.dylib")
elseif (UNIX)
    set(SLANG_LIB_FILE "${SLANG_LIB_DIR}/libslang.so")
endif()

add_library(SlangLib STATIC IMPORTED GLOBAL)
set_target_properties(SlangLib PROPERTIES
        IMPORTED_LOCATION "${SLANG_LIB_FILE}"
        INTERFACE_INCLUDE_DIRECTORIES "${SLANG_INCLUDE_DIR}"
)

# Slang DLLs (runtime)
if (WIN32)
    set(SLANG_DLLS
            "${SLANG_BIN_DIR}/slang.dll"
            "${SLANG_BIN_DIR}/slang-glslang.dll"
    )
endif()

# ----------------------------------------------------
# Common Include / Libs
# ----------------------------------------------------
set(INCLUDE_DIRS
        ${SLANG_INCLUDE_DIR}
        ${VULKAN_SDK}/Include
        ${VULKAN_SDK}/Include/vma
        lib/fastgltf/include
        lib/simdjson/include
        lib/stb
        lib/GLFW/include
        lib/FileWatch
        Projects/PuduCore/lib
        Projects/PuduRenderer/lib
        Projects/CatDiorama/lib
)

set(LIB_DIRS
        ${VULKAN_SDK}/Lib
        lib/fastgltf/${CMAKE_BUILD_TYPE}
        lib/simdjson/${CMAKE_BUILD_TYPE}
        lib/GLFW/Lib
        lib/ktx/lib
)

set(LIBS
        vulkan-1
        fastgltf
        glfw3
        TracyClient
)

# ----------------------------------------------------
# Tracy Setup
# ----------------------------------------------------
set(TRACY_VERSION "v0.12.2")
set(FETCHCONTENT_BASE_DIR "_deps/tracy")
FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG ${TRACY_VERSION}
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(tracy)
include_directories(${tracy_SOURCE_DIR}/public)

# ----------------------------------------------------
# Utility Macros
# ----------------------------------------------------
function(SETUP_TARGET_PROPERTIES target_name)
    set_target_properties(${target_name} PROPERTIES
            CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/testLib/${target_name}/${CMAKE_BUILD_TYPE}
            CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/testLib/${target_name}/${CMAKE_BUILD_TYPE}
            CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${target_name}/${CMAKE_BUILD_TYPE}
    )
    target_include_directories(${target_name} BEFORE PUBLIC ${INCLUDE_DIRS})
    target_link_directories(${target_name} PRIVATE ${LIB_DIRS})

    # Copy Slang DLLs if Windows
    if (WIN32)
        foreach(dll ${SLANG_DLLS})
            add_custom_command(
                    TARGET ${target_name} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${dll}"
                    "$<TARGET_FILE_DIR:${target_name}>"
            )
        endforeach()
    endif()
endfunction()

macro(add_target target_name target_type)
    if (${target_type} STREQUAL STATIC)
        add_library(${target_name} STATIC)
    elseif (${target_type} STREQUAL EXECUTABLE)
        add_executable(${target_name})
    else ()
        message(FATAL_ERROR "Unsupported target type: ${target_type}")
    endif()
    SETUP_TARGET_PROPERTIES(${target_name})
endmacro()

# ----------------------------------------------------
# Core Libraries
# ----------------------------------------------------
add_library(PuduCore STATIC)
target_link_libraries(PuduCore PUBLIC SlangLib)
SETUP_TARGET_PROPERTIES(PuduCore)

set_target_properties(PuduCore PROPERTIES
        COMPILE_DEFINITIONS "SPIRV_REFLECT_USE_SYSTEM_SPIRV_H;IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING;VK_EXT_host_query_reset"
)

file(GLOB_RECURSE PuduCore_SOURCES Projects/PuduCore/**)
target_sources(PuduCore PRIVATE ${PuduCore_SOURCES})
target_link_libraries(PuduCore PRIVATE ${LIBS} KTX::ktx)
target_include_directories(PuduCore PRIVATE ${KTX_INCLUDE_DIRS})

add_target(PuduRenderer STATIC)
set_target_properties(PuduRenderer PROPERTIES
        COMPILE_DEFINITIONS "IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING"
)
file(GLOB_RECURSE PuduRenderer_SOURCES Projects/PuduRenderer/**)
target_sources(PuduRenderer PRIVATE ${PuduRenderer_SOURCES})
target_link_libraries(PuduRenderer PRIVATE PuduCore ${LIBS})

# ----------------------------------------------------
# Application Macro
# ----------------------------------------------------
macro(AddPuduApp app_name)
    add_target(${app_name} EXECUTABLE)
    set_target_properties(${app_name} PROPERTIES
            COMPILE_DEFINITIONS "HLSLPP_FEATURE_TRANSFORM"
    )

    file(GLOB_RECURSE ${app_name}_SOURCES Projects/${app_name}/**)
    target_sources(${app_name} PRIVATE ${${app_name}_SOURCES})
    target_include_directories(${app_name} PRIVATE Projects/${app_name}/Lib)

    target_link_libraries(${app_name} PUBLIC PuduRenderer PuduCore SlangLib)

    # Copy Slang DLLs to app output. Eventually we should copy all needed dll here
    if (WIN32)
        foreach(dll ${SLANG_DLLS})
            add_custom_command(
                    TARGET ${app_name} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${dll}"
                    "$<TARGET_FILE_DIR:${app_name}>"
            )
        endforeach()
    endif()

    set_target_properties(${app_name} PROPERTIES
            DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Assets/"
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Assets/"
    )
endmacro()

# ----------------------------------------------------
# Apps
# ----------------------------------------------------
AddPuduApp(CatDiorama)
AddPuduApp(Test_PBR)
AddPuduApp(Blender_HotReload)
