set(name closestPlane)

file(GLOB src
        *.cpp *.hpp)

add_library(${name}
        SHARED MODULE
        ${src}
        )
target_link_libraries(${name} kengine api)
target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
configure_file(airlines.dat ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/closestPlane/airlines.dat COPYONLY)