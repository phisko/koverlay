cmake_minimum_required(VERSION 3.21)
project(koverlay HOMEPAGE_URL "https://github.com/phisko/koverlay")

set(CMAKE_CXX_STANDARD 20)
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /std:c++20 /DNOMINMAX /MP8")
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

#
# Executable
#

set(exe_name koverlay)

file(GLOB exeFiles
        src/*.cpp src/*.hpp
        src/types/*.cpp src/types/*.hpp)

add_executable(${exe_name} ${exeFiles} ${commonFiles} appicon.rc)
target_include_directories(${exe_name} PRIVATE src)

# set plugin dir
set(runtime_dir $<TARGET_FILE_DIR:${exe_name}>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${runtime_dir}/plugins)

# copy koala.ico
add_custom_command(TARGET koverlay POST_BUILD COMMAND ${CMAKE_COMMAND} -E
        make_directory ${runtime_dir}/resources)
add_custom_command(TARGET koverlay POST_BUILD COMMAND ${CMAKE_COMMAND} -E
        copy
        ${CMAKE_CURRENT_SOURCE_DIR}/koala.ico
        ${runtime_dir}/resources)

# copy example.lua
add_custom_command(TARGET koverlay POST_BUILD COMMAND ${CMAKE_COMMAND} -E
        make_directory ${runtime_dir}/scripts)
add_custom_command(TARGET koverlay POST_BUILD COMMAND ${CMAKE_COMMAND} -E
        copy ${CMAKE_CURRENT_SOURCE_DIR}/examples/example.lua ${runtime_dir}/scripts)

#
# API
#

add_library(api INTERFACE)
target_include_directories(api INTERFACE common)
target_link_libraries(${exe_name} api)

#
# Kengine
#

set(KENGINE_BUILD_TESTS TRUE)
set(KENGINE_NO_TYPE_REGISTRATION TRUE)

# rendering
set(KENGINE_GLFW TRUE)
set(KENGINE_OPENGL TRUE)
set(KENGINE_OPENGL_NO_DEBUG_TOOLS TRUE)
set(KENGINE_OPENGL_NO_DEFAULT_SHADERS TRUE)

# log
set(KENGINE_LOG_FILE TRUE)
set(KENGINE_LOG_IMGUI TRUE)
set(KENGINE_LOG_STDOUT TRUE)
set(KENGINE_LOG_VISUAL_STUDIO TRUE)

# imgui
set(KENGINE_IMGUI_ADJUSTABLE TRUE)
set(KENGINE_IMGUI_PROMPT TRUE)
set(KENGINE_IMGUI_TOOL TRUE)

# scripting
set(KENGINE_LUA TRUE)
set(KENGINE_PYTHON TRUE)

# Unused
# set(KENGINE_ASSIMP TRUE)
# set(KENGINE_BULLET TRUE)
# set(KENGINE_COLLISION TRUE)
# set(KENGINE_IMGUI_ENGINE_STATS TRUE)
# set(KENGINE_IMGUI_ENTITY_EDITOR TRUE)
# set(KENGINE_IMGUI_ENTITY_SELECTOR TRUE)
# set(KENGINE_INPUT TRUE)
# set(KENGINE_KINEMATIC TRUE)
# set(KENGINE_MODEL_CREATOR TRUE)
# set(KENGINE_ONCLICK TRUE)
# set(KENGINE_OPENGL_SPRITES TRUE)
# set(KENGINE_POLYVOX TRUE)
# set(KENGINE_RECAST TRUE)
# set(KENGINE_SFML TRUE)

add_subdirectory(kengine)
target_link_libraries(${exe_name} kengine)

#
# Plugins
#

add_subdirectory(plugins)

# examples

file(GLOB children examples/*)
foreach(child ${children})
    if(IS_DIRECTORY ${child})
        add_subdirectory(${child})
    endif()
endforeach()

#
# Installer
#

install(IMPORTED_RUNTIME_ARTIFACTS ${exe_name} FRAMEWORK
        COMPONENT core)
install(FILES koala.ico
        DESTINATION bin/resources
        COMPONENT core)
install(TARGETS controller
        DESTINATION bin/plugins
        COMPONENT core)

install(DIRECTORY examples
        DESTINATION .
        COMPONENT examples)
install(FILES examples/example.lua
        DESTINATION bin/scripts
        COMPONENT examples)

install(TARGETS kengine
        DESTINATION sources/lib
        COMPONENT sources)
install(TARGETS putils_imgui
        DESTINATION sources/lib
        COMPONENT sources)
install(DIRECTORY common
        DESTINATION sources
        COMPONENT sources)
install(DIRECTORY kengine
        DESTINATION sources
        COMPONENT sources)

include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Koala Overlay: a framework for ImGui tools")
set(CPACK_PACKAGE_VENDOR "Nicolas Phister")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_NSIS_INSTALLED_ICON_NAME ${exe_name})
set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\koala.ico")
set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\koala.ico")
set(CPACK_NSIS_HELP_LINK ${CMAKE_PROJECT_HOMEPAGE_URL})
set(CPACK_NSIS_URL_INFO_ABOUT ${CMAKE_PROJECT_HOMEPAGE_URL})
set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_PACKAGE_EXECUTABLES ${exe_name} ${exe_name})

include(CPack)
cpack_add_component(core REQUIRED
        DESCRIPTION "The core executable required for the framework")
cpack_add_component(examples
        DESCRIPTION "Sample Lua script and sources for a plugin and kengine system")
cpack_add_component(sources
        DESCRIPTION "The source code required to build new plugins and systems")
