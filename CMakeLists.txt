cmake_minimum_required(VERSION 3.14)

project(gdfe
        VERSION 0.1.0
        DESCRIPTION "Not really a game engine..."
        LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_subdirectory(external/vma)

set(NO_WARNING_FLAGS "-Wno-deprecated-declarations -Wno-unused-parameter -Wno-missing-braces -Wno-unused-variable -Wno-ignored-qualifiers -Wno-missing-field-initializers")

set(CMAKE_C_FLAGS "-Wall -W -Wextra ${NO_WARNING_FLAGS} -ffast-math -march=native -mavx2 -fcolor-diagnostics")

function(find_src_files result search_dir)
    file(GLOB_RECURSE source_files
        "${search_dir}/*.c"
    )
    set(${result} ${source_files} PARENT_SCOPE)
endfunction()

find_src_files(GDFE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src")
find_src_files(GDFE_EXTERNAL "${CMAKE_CURRENT_SOURCE_DIR}/external")

SET_SOURCE_FILES_PROPERTIES(${GDFE_SRC} ${GDFE_EXTERNAL} PROPERTIES LANGUAGE C)

add_library(${PROJECT_NAME} ${GDFE_SRC} ${GDFE_EXTERNAL})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(PROFILE_DIR "release")
    set(SHADERC_LIB_NAME shaderc_combined)
    target_compile_definitions(${PROJECT_NAME} PRIVATE GDF_RELEASE NDEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -fno-plt -fuse-ld=lld -funroll-loops -finline-functions -fomit-frame-pointer -fno-exceptions -ftree-vectorize -fvectorize -fslp-vectorize")
else()
    set(PROFILE_DIR "debug")
    set(SHADERC_LIB_NAME shaderc_combinedd)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -Og -fno-omit-frame-pointer")
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE GDF_COLLECTIONS)

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
    NO_DEFAULT_PATH
)

if(NOT SHADERC_LIBRARY)
    message(FATAL_ERROR "Could not find shaderc library in Vulkan SDK")
endif()

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Vulkan::Vulkan
        ${SHADERC_LIBRARY}
)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/external
        ${CMAKE_CURRENT_SOURCE_DIR}/src/internal
)
