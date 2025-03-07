cmake_minimum_required(VERSION 3.14)

project(gdfe
        VERSION 0.1.0
        DESCRIPTION "Not really a game engine..."
        LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

set(NO_WARNING_FLAGS "-Wno-deprecated-declarations -Wno-unused-parameter -Wno-missing-braces -Wno-unused-variable -Wno-ignored-qualifiers -Wno-missing-field-initializers")

set(CMAKE_C_FLAGS "-Wall -W -Wextra ${NO_WARNING_FLAGS} -ffast-math -march=native -mavx2 -fcolor-diagnostics")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(PROFILE_DIR "release")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -fno-plt -fuse-ld=lld -funroll-loops -finline-functions -fomit-frame-pointer -fno-exceptions -ftree-vectorize -fvectorize -fslp-vectorize -DNDEBUG -DGDF_RELEASE")
else()
    set(PROFILE_DIR "debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Og -DDEBUG")
endif()

function(find_src_files result search_dir)
    file(GLOB_RECURSE source_files
        "${search_dir}/*.c"
    )
    set(${result} ${source_files} PARENT_SCOPE)
endfunction()

find_src_files(SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src")
find_src_files(EXTERN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/extern")
add_library(${PROJECT_NAME} ${SOURCE_FILES} ${EXTERN_FILES}
        include/gdfe.h
        src/gdfe.c
        src/input/internal.h
        src/os/window_internal.h
        src/render/core_renderer.h
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Vulkan::Vulkan
)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/extern
)
