cmake_minimum_required(VERSION 3.12)
project(Hobbit_game C)

set(CMAKE_C_STANDARD 99)

add_executable(Hobbit_game main.c)

add_executable(Hobbit_client client.c)