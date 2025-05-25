cmake_minimum_required (VERSION 3.15)
project(game)




add_executable(${PROJECT_NAME}
    grassfield.cpp
    main.cpp
)

#set(raylib_VERBOSE 1)
target_link_libraries(${PROJECT_NAME} raylib)
