cmake_minimum_required(VERSION 3.25)

project(chippy8)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES 
    ../chipper.cpp
    tehBEEP.cpp
    tehBOOP.cpp
    tehBUS.cpp
    tehCHIP.cpp
    tehCPUS.cpp
    tehRAMS.cpp
    tehROM.cpp
    chipperSDL.cpp

)

# 1. Look for a SDL2 package, 2. look for the SDL2 component and 3. fail if none can be found
find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
# 1. Look for a SDL2 package, 2. Look for the SDL2maincomponent and 3. DO NOT fail when SDL2main is not available
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)

add_executable(chippy8 ${SOURCE_FILES})

# TODO: Handle other platforms
IF (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_custom_target (BuildNFDLib ALL
        COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR}/lib/nfd/build/gmake_windows/
    )
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL "Darwin") 
    add_custom_target (BuildNFDLib ALL
        COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR}/lib/nfd/build/gmake_macosx/
    )
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    SET(SYSTEM_LIBS
        ${COCOA_LIBRARY}
    )
ELSE ()
    add_custom_target (BuildNFDLib ALL
        COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR}/lib/nfd/build/gmake_linux_zenity/
    )
ENDIF()

# This is all *really* messy but it works!
add_library(NFDLib STATIC IMPORTED)

IF(CMAKE_SYSTEM_NAME STREQUAL "Windows") 
    set_target_properties(NFDLib PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/nfd/build/lib/Release/x64/nfd.lib
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/lib/nfd/src/include
        INTERFACE_COMPILE_DEFINITIONS "USING_NFDLIB;NFDLIB_STATIC"
    )
ELSE()
    set_target_properties(NFDLib PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/nfd/build/lib/Release/x64/libnfd.a
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/lib/nfd/src/include
        INTERFACE_COMPILE_DEFINITIONS "USING_NFDLIB;NFDLIB_STATIC"
    )
ENDIF()   

add_dependencies(NFDLib BuildNFDLib)

# SDL2::SDL2main is required for windows GUI stuff. It may, or may not exist depending on system.
IF(TARGET SDL2::SDL2main) 
    # Docmentation says it's dependent on SDL2, so it needs to be added before SDL2::SDL2
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2main)
ENDIF()

target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 NFDLib ${SYSTEM_LIBS})
