cmake_minimum_required(VERSION 3.14)

project(gdfe
        VERSION 0.1.0
        DESCRIPTION "Not really a game engine..."
        LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

add_subdirectory(external/vma)

#set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "Enable SPIRV-Cross tests." FORCE)
set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "Build spirv-reflect executable" FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON CACHE BOOL "Build a SPIRV-Reflect static library" FORCE)
add_subdirectory(external/spirv_reflect)

set(NO_WARNING_FLAGS "-Wno-deprecated-declarations -Wno-unused-parameter -Wno-missing-braces -Wno-unused-variable -Wno-ignored-qualifiers -Wno-missing-field-initializers")

set(CMAKE_C_FLAGS "-Wall -W -Wextra ${NO_WARNING_FLAGS} -ffast-math -march=native -mavx2 -fcolor-diagnostics")

function(find_src_files result search_dir)
    file(GLOB_RECURSE source_files
        "${search_dir}/*.c"
    )
    set(${result} ${source_files} PARENT_SCOPE)
endfunction()

find_src_files(GDFE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src")
#find_src_files(GDFE_EXTERNAL "${CMAKE_CURRENT_SOURCE_DIR}/external")

SET_SOURCE_FILES_PROPERTIES(${GDFE_SRC} ${GDFE_EXTERNAL} PROPERTIES LANGUAGE C)

add_library(gdfe ${GDFE_SRC} ${GDFE_EXTERNAL}
        src/render/vk_alloc.c)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(PROFILE_DIR "release")
    target_compile_definitions(gdfe PRIVATE GDF_RELEASE NDEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -fno-plt -fuse-ld=lld -funroll-loops -finline-functions -fomit-frame-pointer -fno-exceptions -ftree-vectorize -fvectorize -fslp-vectorize")
else()
    set(PROFILE_DIR "debug")
    target_compile_definitions(gdfe PRIVATE DEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -Og -fno-omit-frame-pointer")
endif()

# Platform specific libraries to be linked
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM_LIBS user32 gdi32 kernel32)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PLATFORM_LIBS X11 Xi Xrandr stdc++ m rt pthread)
endif()

# weird ahh naming conventions
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(SHADERC_LIB_NAME shaderc_combined)
    else()
        set(SHADERC_LIB_NAME shaderc_combinedd)
    endif()
else()
    set(SHADERC_LIB_NAME shaderc_shared)
endif()

target_compile_definitions(gdfe PRIVATE GDF_COLLECTIONS)

if(DEFINED ENV{VULKAN_SDK})
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(Vulkan_INCLUDE_DIRS "$ENV{VULKAN_SDK}/Include")
        set(Vulkan_LIBRARY_DIRS "$ENV{VULKAN_SDK}/Lib")
    else()
        set(Vulkan_INCLUDE_DIRS "$ENV{VULKAN_SDK}/include")
        set(Vulkan_LIBRARY_DIRS "$ENV{VULKAN_SDK}/lib")
    endif()
    include_directories(${Vulkan_INCLUDE_DIRS})
    find_package(Vulkan REQUIRED)
else()
    find_package(Vulkan REQUIRED)
    include_directories(${Vulkan_INCLUDE_DIRS})
endif()

# find shaderc
find_library(SHADERC_LIBRARY
    NAMES ${SHADERC_LIB_NAME}
    PATHS ${Vulkan_LIBRARY_DIRS}
)

message("searched ${Vulkan_LIBRARY_DIRS}")
message("lf ${SHADERC_LIB_NAME}")
message("got ${SHADERC_LIBRARY}")

if(NOT SHADERC_LIBRARY)
    message(FATAL_ERROR "Could not find shaderc library.")
endif()

target_link_libraries(gdfe
        PRIVATE
        Vulkan::Vulkan
        ${PLATFORM_LIBS}
        ${SHADERC_LIBRARY}
        spirv-reflect-static
)

target_include_directories(gdfe
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)

target_include_directories(gdfe
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/external
        ${CMAKE_CURRENT_SOURCE_DIR}/src/internal
)

# build examples
add_executable(example_bouncing examples/bouncing/main.c)
target_link_libraries(example_bouncing gdfe)

add_executable(example_io examples/io/main.c)
target_link_libraries(example_io gdfe)

# copy resources dir
file(GLOB_RECURSE RESOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/*")

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resources
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources
        DEPENDS ${RESOURCE_FILES}
        COMMENT "Copying resources directory to build directory"
)

add_custom_target(copy_resources ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/resources)

add_dependencies(gdfe copy_resources)
