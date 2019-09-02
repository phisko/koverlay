cmake_minimum_required(VERSION 3.10)
project(koverlay) 

set(CMAKE_CXX_STANDARD 17)
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /std:c++17 /DNOMINMAX")
endif()

set(libdir "${PROJECT_SOURCE_DIR}/lib")
set(bindir "${PROJECT_SOURCE_DIR}/bin")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${libdir})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${libdir}/debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${libdir}/release)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${libdir}/relwithdebinfo)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${libdir}/minsizerel)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${bindir})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${bindir}/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${bindir}/release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${bindir}/relwithdebinfo)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${bindir}/minsizerel)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${bindir}/plugins)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${bindir}/debug/plugins)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${bindir}/release/plugins)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${bindir}/relwithdebinfo/plugins)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${bindir}/minsizerel/plugins)

#
# Actual project
#

## Kengine

set(KENGINE_LUA TRUE)
set(KENGINE_PYTHON TRUE)
set(KENGINE_IMGUI_OVERLAY TRUE)
add_subdirectory(kengine)

## API
add_library(api INTERFACE)
target_include_directories(api INTERFACE common)

## Executable

set(exe_name koverlay)

file(GLOB exeFiles src/*.cpp src/*.hpp
        src/components/*.cpp src/components/*.hpp
        src/gameobjects/*.cpp src/gameobjects/*.hpp
        src/systems/*.cpp src/systems/*.hpp)

add_executable(${exe_name} ${exeFiles} ${commonFiles})
target_include_directories(${exe_name} PRIVATE src)
target_link_libraries(${exe_name} kengine api)

add_subdirectory(plugins)
