set(name MY_PLUGIN)

file(GLOB src *.cpp *.hpp imgui/*.cpp imgui/*.hpp)
add_library(${name} SHARED MODULE ${src})
target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_LIST_DIR} imgui)
