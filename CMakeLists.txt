cmake_minimum_required(VERSION 3.16)
project(GJK_NonConvex VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

add_executable(gjk_demo
    src/main.cpp
)

target_include_directories(gjk_demo PRIVATE
    src/algorithms
    src/data_structures
    ${SDL2_INCLUDE_DIRS}
)
target_link_libraries(gjk_demo PRIVATE ${SDL2_LIBRARIES})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

target_compile_options(gjk_demo PRIVATE
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Debug>:-g>
)

# ── Optional test target ────────────────────────────────────────────
add_executable(test_algorithms
    tests/test_algorithms.cpp
)
target_include_directories(test_algorithms PRIVATE
    src/algorithms
    src/data_structures
)
