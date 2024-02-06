cmake_minimum_required(VERSION 3.24)
project(rogue VERSION 0.0.1 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

find_package(Git REQUIRED)

include(FetchContent)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON CACHE BOOL "Prevents cmake -S. -B.")
set(CMAKE_DISABLE_SOURCE_CHANGES ON CACHE BOOL "Prevent writing files to CMAKE_SOURCE_DIR under configure")

FetchContent_Declare(daw
  GIT_REPOSITORY https://codeberg.org/onelin/daw.git
  GIT_TAG glfw
)

add_compile_options(-Og -ggdb3 -fno-omit-frame-pointer)
add_link_options(-Og -ggdb3 -fno-omit-frame-pointer)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-D_DEBUG)
endif()

FetchContent_MakeAvailable(daw)

daw_add_state(mainstate)

add_executable(${PROJECT_NAME} src/main.c)
target_link_libraries(${PROJECT_NAME} PUBLIC daw)
target_include_directories(${PROJECT_NAME} PUBLIC include)
