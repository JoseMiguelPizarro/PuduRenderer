cmake_minimum_required(VERSION 3.21)

# Define the workspace/project
project(PuduRenderer LANGUAGES CXX)
include(FetchContent)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set architecture
set(CMAKE_GENERATOR_PLATFORM x64)

# Define configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs")

# Set Vulkan SDK
set(VULKAN_SDK $ENV{VULKAN_SDK})

# Fetch KTX
find_package(KTX REQUIRED)

# Include directories
set(INCLUDE_DIRS
    ${VULKAN_SDK}/Include
    ${VULKAN_SDK}/Include/vma
    lib/dxc/include
    lib/fastgltf/include
    lib/simdjson/include
    lib/boolinq
    lib/SPIRV_Reflect/Include
    lib/stb
    lib/GLFW/include
    lib/fmt/include
    lib/tiny_obj_loader
    Projects/PuduCore/lib
    Projects/PuduRenderer/lib
	Projects/App/lib
)
include_directories(${INCLUDE_DIRS})

# Library directories
set(LIB_DIRS
    ${VULKAN_SDK}/Lib
    lib/dxc/lib
    lib/fastgltf/${CMAKE_BUILD_TYPE}
    lib/simdjson/${CMAKE_BUILD_TYPE}
    lib/SPIRV_Reflect/${CMAKE_BUILD_TYPE}
    lib/GLFW/Lib
    lib/fmt/${CMAKE_BUILD_TYPE}
    lib/ktx/lib
)
link_directories(${LIB_DIRS})

# Libraries
set(LIBS
    vulkan-1
    dxcompiler
    fastgltf
    SPIRV-Reflect
    glfw3
    fmtd
)

set(SLANG_VERSION "2025.2.2")
if(WIN32)
    set(SLANG_OS "windows-x86_64")
elseif(APPLE)
    set(SLANG_OS "macos-aarch64")
elseif(UNIX)
    set(SLANG_OS "linux-x86_64")
endif()

include(FetchContent)
FetchContent_Populate(slang_zip URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-${SLANG_OS}.zip QUIET)
message(STATUS "Slang dir: ${slang_zip_SOURCE_DIR}")
add_library(slang SHARED IMPORTED GLOBAL)
if(WIN32)
    set_target_properties(slang PROPERTIES
            SLANG_DLL ${slang_zip_SOURCE_DIR}/bin/slang.dll
            GLSLANG_DLL ${slang_zip_SOURCE_DIR}/bin/slang-glslang.dll
            IMPORTED_IMPLIB ${slang_zip_SOURCE_DIR}/lib/slang.lib
            INTERFACE_INCLUDE_DIRECTORIES ${slang_zip_SOURCE_DIR}/include
    )
elseif(APPLE)
    set_target_properties(slang PROPERTIES
            IMPORTED_LOCATION ${slang_zip_SOURCE_DIR}/lib/libslang.dylib
            IMPORTED_LOCATION ${slang_zip_SOURCE_DIR}/lib/libslang-glslang.dylib
            INTERFACE_INCLUDE_DIRECTORIES ${slang_zip_SOURCE_DIR}/include
    )
elseif(UNIX)
    set_target_properties(slang PROPERTIES
            IMPORTED_LOCATION ${slang_zip_SOURCE_DIR}/lib/libslang.so
            IMPORTED_LOCATION ${slang_zip_SOURCE_DIR}/lib/libslang-glslang.so
            INTERFACE_INCLUDE_DIRECTORIES ${slang_zip_SOURCE_DIR}/include
    )
endif()

function(SETUP_TARGET_PROPERTIES target_name)

    set_target_properties(${target_name} PROPERTIES
            CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/testLib/${target_name}/${CMAKE_BUILD_TYPE}
            CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/testLib/${target_name}/${CMAKE_BUILD_TYPE}
            CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${target_name}/${CMAKE_BUILD_TYPE}
    )
    message(STATUS  ${CMAKE_BINARY_DIR}/bin/${target_name}/${CMAKE_BUILD_TYPE})
    target_include_directories(${target_name} PUBLIC ${INCLUDE_DIRS})
    target_link_directories(${target_name} PRIVATE ${LIB_DIRS})
endfunction()
# Helper macros
macro(add_target target_name target_type)
    if(${target_type} STREQUAL STATIC)
        add_library(${target_name} STATIC)
    elseif(${target_type} STREQUAL EXECUTABLE)
        add_executable(${target_name})
    else()
        message(FATAL_ERROR "Unsupported target type: ${target_type}")
    endif()
    set_target_properties(${target_name} PROPERTIES
        CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/testLib/${target_name}/${CMAKE_BUILD_TYPE}
        CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/testLib/${target_name}/${CMAKE_BUILD_TYPE}
        CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${target_name}/${CMAKE_BUILD_TYPE}
    )
    message(STATUS  ${CMAKE_BINARY_DIR}/bin/${target_name}/${CMAKE_BUILD_TYPE})
    target_include_directories(${target_name} PUBLIC ${INCLUDE_DIRS})
    target_link_directories(${target_name} PRIVATE ${LIB_DIRS})
endmacro()


# Add projects
add_library(PuduCore STATIC
        Projects/PuduCore/Src/Enums/ResourceUsage.h)
SETUP_TARGET_PROPERTIES(PuduCore)

set_target_properties(PuduCore PROPERTIES
    COMPILE_DEFINITIONS "SPIRV_REFLECT_USE_SYSTEM_SPIRV_H;IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING"
)
target_link_libraries(PuduCore PUBLIC slang)


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

add_target(App EXECUTABLE)
set_target_properties(App PROPERTIES
    COMPILE_DEFINITIONS "HLSLPP_FEATURE_TRANSFORM"
)
file(GLOB_RECURSE App_SOURCES Projects/App/**)
target_sources(App PRIVATE ${App_SOURCES})

target_link_libraries(App PUBLIC PuduRenderer PuduCore PUBLIC slang)

get_target_property(slangDLL slang SLANG_DLL)
get_target_property(glslangDLL slang GLSLANG_DLL)
set(buildDir "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/")

file(MAKE_DIRECTORY ${buildDir})

configure_file(${slangDLL} "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/" COPYONLY)

configure_file(${glslangDLL} "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/" COPYONLY)

message(STATUS "Copying asset files")
add_custom_command(
        TARGET App
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Assets"
        "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/Assets/"
)