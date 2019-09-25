cmake_minimum_required(VERSION 3.5)

project(3531 
    LANGUAGES CXX
    VERSION 2.0.0)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER  arm-hisiv500-linux-gcc)
SET(CMAKE_CXX_COMPILER arm-hisiv500-linux-g++)

set(VER "V2.1.1")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_SKIP_RPATH true)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(DEBUG false)

if(DEBUG) 
set(HEADER "DEBUG")
else()
set(HEADER "RELEASE")
endif()

string(CONCAT VER ${VER} "_")
string(CONCAT VER ${VER} ${HEADER})
string(CONCAT VER ${VER} "_3532_")
string(CONCAT VER ${VER} ${CHIP_TYPE})
string(TIMESTAMP TS "_%Y-%m-%dT%H:%M:%S")
string(CONCAT RS_VERSION ${VER} ${TS})
message(STATUS ${RS_VERSION})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")

if(DEBUG)
    message(STATUS "BUILD DEBUG")
    set(CMAKE_CXX_FLAGS "-g -Wall ${CMAKE_CXX_FLAGS} -DCHIP_TYPE=${CHIP_TYPE} -DRS_VERSION=${RS_VERSION}")
    set(CMAKE_C_FLAGS "-g -Wall ${CMAKE_C_FLAGS} -DCHIP_TYPE=${CHIP_TYPE} -DRS_VERSION=${RS_VERSION}")
else()
    message(STATUS "BUILD RELEASE")
    set(CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS} -DCHIP_TYPE=${CHIP_TYPE} -DRS_VERSION=${RS_VERSION}")
    set(CMAKE_C_FLAGS "-Wall ${CMAKE_C_FLAGS} -DCHIP_TYPE=${CHIP_TYPE} -DRS_VERSION=${RS_VERSION}")
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/hbrs
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/include
)

link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/lib
)

add_subdirectory(hbrs)
