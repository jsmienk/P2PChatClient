cmake_minimum_required(VERSION 3.6)
project(LoRaP2PNetwork)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp P2PClient.cpp P2PClient.h)
add_executable(LoRaP2PNetwork ${SOURCE_FILES})