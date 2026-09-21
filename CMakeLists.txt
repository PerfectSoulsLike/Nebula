cmake_minimum_required(VERSION 3.15)
project(NebulaClient CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Создаем инжектор (.exe)
add_executable(NebulaInjector main.cpp)
# Подключаем к инжектору библиотеку Advapi32 для работы функции grantAppContainerAccess
target_link_libraries(NebulaInjector PRIVATE Advapi32)

# Создаем саму визуальную DLL
add_library(NebulaClient SHARED nebula.cpp)
