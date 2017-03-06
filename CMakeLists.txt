cmake_minimum_required(VERSION 3.6)
project(LoRaP2PNetwork)

file(GLOB crypto_SRC
        "cryptopp565/*.h"
        "cryptopp565/*.cpp"
        )

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp P2PRouter.cpp P2PRouter.h ${crypto_SRC})
add_executable(LoRaP2PNetwork ${SOURCE_FILES})
