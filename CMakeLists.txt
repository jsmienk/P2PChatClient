cmake_minimum_required(VERSION 3.6)
project(LoRaP2PNetwork)

file(GLOB crypto_SRC
        "cryptopp565/*.h"
        "cryptopp565/*.cpp"
        )

file(GLOB rapidjson_SRC
        "rapidjson/*.h"
        "rapidjson/*.cpp"
        )

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp ${crypto_SRC} ${rapidjson_SRC})
add_executable(LoRaP2PNetwork ${SOURCE_FILES})

if(WIN32)
    target_link_libraries(LoRaP2PNetwork wsock32 ws2_32)
endif()