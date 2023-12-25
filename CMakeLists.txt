cmake_minimum_required(VERSION 3.5)
project(tradingsystem)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# If on windows, uncomment and set the path to the boost library
set(CMAKE_PREFIX_PATH ""C:\Users\Chemi\OneDrive\Desktop\Work\boost_1_83_0"")

# Find the Boost library
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

# trading system executable
add_executable(tradingsystem main.cpp)
target_link_libraries(tradingsystem ${Boost_LIBRARIES})
