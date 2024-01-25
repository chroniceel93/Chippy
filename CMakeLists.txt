cmake_minimum_required(VERSION 3.25)

project(chippy8)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES 
    ../chipper.cpp
    tehBEEP.cpp
    tehBOOP.cpp
    tehBUS.cpp
    tehCHIP.cpp
    tehCPUS.cpp
    tehRAMS.cpp
    tehROM.cpp
    tehSCREEN.cpp
)

find_package(SDL2 REQUIRED)

add_executable(chippy8 ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2-static)