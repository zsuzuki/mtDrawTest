cmake_minimum_required(VERSION 3.11)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-g")

project(mtDrawTest)

find_package(PkgConfig REQUIRED)
find_package(sdl2 CONFIG REQUIRED)
pkg_check_modules(sdl2 REQUIRED sdl2 SDL2_ttf)
pkg_search_module(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)
find_package(Boost 1.70.0 REQUIRED COMPONENTS chrono exception filesystem thread random system)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    /usr/local/include
    ${OPENSSL_INCLUDE_DIR})

link_directories(/usr/local/lib)

set(src
    src/draw.cpp
    src/main.cpp
)

add_executable(${PROJECT_NAME} ${src})

target_compile_options(${PROJECT_NAME} PUBLIC ${SDL2_CFLAGS_OTHER})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ${Boost_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2TTF_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${libs})
