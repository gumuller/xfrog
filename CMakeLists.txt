cmake_minimum_required(VERSION 3.24)
project(xfrog VERSION 1.3 LANGUAGES C CXX)

if(WIN32 AND NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "Xfrog requires 32-bit Windows. Configure with -A Win32.")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(WIN32)
    include(FetchContent)
    set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(ZLIB_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(ZLIB_BUILD_STATIC ON CACHE BOOL "" FORCE)
    set(ZLIB_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(zlib
        URL https://zlib.net/fossils/zlib-1.3.2.tar.gz
        URL_HASH SHA256=bb329a0a2cd0274d05519d61c667c062e06990d72e125ee2dfa8de64f0119d16
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(zlib)
    set(XFROG_ZLIB zlibstatic)
else()
    find_package(ZLIB REQUIRED)
    find_package(SDL3 3.2 REQUIRED CONFIG)
    set(XFROG_ZLIB ZLIB::ZLIB)
endif()

set(GAME_SRC src/game/src)
set(GAME_COMMON
    ${GAME_SRC}/car.cpp ${GAME_SRC}/cfs.cpp ${GAME_SRC}/engine.cpp
    ${GAME_SRC}/ent.cpp ${GAME_SRC}/error.cpp ${GAME_SRC}/font.cpp
    ${GAME_SRC}/game.cpp ${GAME_SRC}/highscore.cpp ${GAME_SRC}/hud.cpp
    ${GAME_SRC}/item.cpp ${GAME_SRC}/mem_man.cpp
    ${GAME_SRC}/menu.cpp ${GAME_SRC}/player.cpp
    ${GAME_SRC}/tga.cpp ${GAME_SRC}/thief.cpp
)
if(WIN32)
    add_executable(xfrog WIN32 ${GAME_COMMON}
        ${GAME_SRC}/input.cpp ${GAME_SRC}/sound.cpp ${GAME_SRC}/timer.cpp
        ${GAME_SRC}/video.cpp ${GAME_SRC}/xfrog_main.cpp)
else()
    set(GAME_SDL ${GAME_SRC}/sdl/input.cpp ${GAME_SRC}/sdl/sound.cpp
        ${GAME_SRC}/sdl/timer.cpp ${GAME_SRC}/sdl/video.cpp ${GAME_SRC}/sdl/platform.cpp)
    add_library(xfrog-core STATIC ${GAME_COMMON} ${GAME_SDL})
    target_compile_features(xfrog-core PUBLIC cxx_std_11)
    target_compile_definitions(xfrog-core PUBLIC XFROG_SDL)
    target_include_directories(xfrog-core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${GAME_SRC}")
    target_link_libraries(xfrog-core PUBLIC SDL3::SDL3 ${XFROG_ZLIB})
    target_compile_options(xfrog-core PRIVATE -Wall -Wextra -Wno-write-strings -Wno-unused-variable)
    add_executable(xfrog ${GAME_SRC}/sdl/main.cpp)
    target_link_libraries(xfrog PRIVATE xfrog-core)
endif()
target_compile_features(xfrog PRIVATE cxx_std_11)
target_link_libraries(xfrog PRIVATE ${XFROG_ZLIB})
if(WIN32)
    target_compile_definitions(xfrog PRIVATE
        NOMINMAX DIRECTINPUT_VERSION=0x0800
        DIRECTDRAW_VERSION=0x0700 _CRT_SECURE_NO_WARNINGS
    )
    target_link_libraries(xfrog PRIVATE ddraw dinput8 dxguid dsound winmm user32 gdi32)
endif()
if(MSVC)
    target_compile_options(xfrog PRIVATE /W3)
endif()
set_target_properties(xfrog PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:xfrog>"
)
add_custom_command(TARGET xfrog POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
        "-DOUTPUT_DIR=$<TARGET_FILE_DIR:xfrog>"
        "-DZLIB_SOURCE_DIR=${zlib_SOURCE_DIR}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/StageRuntime.cmake"
    VERBATIM
)

include(CTest)
if(BUILD_TESTING AND NOT WIN32)
    add_subdirectory(tests)
endif()
