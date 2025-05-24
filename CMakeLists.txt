cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+

set(CMAKE_CXX_STANDARD 20)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# Dependencies
#set(RAYLIB_VERSION 5.5)

# Add Raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games
set(GRAPHICS       GRAPHICS_API_OPENGL_43 CACHE STRING "" FORCE) # don't build the supplied example games

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG 8d9c1cecb7f53aef720e2ee0d1558ffc39fa7eef
    GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(raylib)

function(PRINT_VARIABLES)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endfunction()



add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
#add_compile_options($<$<CXX_COMPILER_ID:MSVC>-ftime-ftrace)
if (WIN32)
    ADD_DEFINITIONS(-DWIN32_LEAN_AND_MEAN=1)
endif (WIN32)






# Our Project

add_subdirectory("src")


## Web Configurations
#if (${PLATFORM} STREQUAL "Web")
#    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html") # Tell Emscripten to build an example.html file.
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1")
#endif()
#
## Checks if OSX and links appropriate frameworks (Only required on MacOS)
#if (APPLE)
#    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
#    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
#    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
#endif()