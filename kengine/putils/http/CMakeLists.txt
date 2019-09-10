file(GLOB SOURCE_FILES
        *.cpp
        *.hpp
        modules/*.cpp
        modules/*.hpp
        )

add_library(http STATIC ${SOURCE_FILES})
target_include_directories(http
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}
        PRIVATE src lib)
target_link_libraries(http mediator connection)

if (UNIX)
    target_link_libraries(http pthread)
endif ()

if (WIN32)
    target_link_libraries(http wsock32 ws2_32)
endif ()

# include(cotire)
# cotire(http)
