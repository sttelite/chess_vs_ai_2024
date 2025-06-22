cmake_minimum_required(VERSION 3.2)
project(chessvsAI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2 COMPONENTS graphics audio REQUIRED)

set(SFML_DLL_PATH "${SFML_DIR}/../../../bin/")

add_executable(chessvsAI main.cpp)

target_link_libraries(chessvsAI sfml-system sfml-window sfml-graphics)

add_custom_command(TARGET chessvsAI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources/ ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(TARGET chessvsAI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SFML_DLL_PATH} ${CMAKE_CURRENT_BINARY_DIR})