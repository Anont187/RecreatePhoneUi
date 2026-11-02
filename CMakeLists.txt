cmake_minimum_required(VERSION 3.20)
project(FieldKitLauncher LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glfw3 3.3 REQUIRED)
find_package(Freetype REQUIRED)
find_package(Threads REQUIRED)

add_executable(field_kit main.cpp glad.c)
target_include_directories(field_kit PRIVATE include glm)
target_compile_definitions(field_kit PRIVATE GLFW_INCLUDE_NONE)
target_link_libraries(field_kit PRIVATE glfw Freetype::Freetype Threads::Threads ${CMAKE_DL_LIBS})