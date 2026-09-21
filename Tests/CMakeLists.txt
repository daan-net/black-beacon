cmake_minimum_required(VERSION 3.16)
project(BB_LogicTests CXX)

# Builds and runs the engine-free logic layer of BLACK BEACON.
# Usage: cmake -S Tests -B Tests/build && cmake --build Tests/build && Tests/build/bb_logic_tests

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BB_LOGIC_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../Source/BlackBeacon/Private/BlackBeacon/Logics/BBBeamMath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Source/BlackBeacon/Private/BlackBeacon/Logics/BBRevealStateMachine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Source/BlackBeacon/Private/BlackBeacon/Logics/BBObjectiveGraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Source/BlackBeacon/Private/BlackBeacon/Logics/BBWeatherState.cpp
)

add_executable(bb_logic_tests
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${BB_LOGIC_SOURCES}
)

target_include_directories(bb_logic_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../Source/BlackBeacon/Public
)