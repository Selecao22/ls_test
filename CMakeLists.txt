cmake_minimum_required(VERSION 3.17)
project(my_ls C)

set(CMAKE_C_STANDARD 99)

add_executable(my_ls main.c file_info.c file_info.h dir_info.c dir_info.h)