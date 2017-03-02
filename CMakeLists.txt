cmake_minimum_required(VERSION 3.6)
project(LoRaP2PClient)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(LoRaP2PClient ${SOURCE_FILES})