cmake_minimum_required(VERSION 3.10)
project(tilemap-editor)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

file(GLOB_RECURSE SOURCES 
    src/main.cpp
    thirdparty/imgui/*.cpp
    thirdparty/imgui/backends/*.cpp
)
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "thirdparty/imgui"
    "thirdparty/imgui/backends"
)
target_link_libraries(${PROJECT_NAME} SDL2 SDL2_image)