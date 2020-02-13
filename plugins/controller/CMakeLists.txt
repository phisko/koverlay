set(name controller)

file(GLOB src
        *.cpp *.hpp)

add_library(${name}
        SHARED MODULE
        ${src}
        )
target_link_libraries(${name} kengine)
target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
