#CMake basics
cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0048 NEW)

# set the project name
project(sync VERSION 1.0.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set the standards/options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# other settings
set(CMAKE_INSTALL_MESSAGE LAZY)
get_filename_component(my_path "~/.sync" ABSOLUTE)
set(CMAKE_INSTALL_PREFIX ${my_path})

# set binary names
set(GUI_NAME ${PROJECT_NAME}gui)
set(CLI_NAME ${PROJECT_NAME}cli)
set(LIB_NAME ${PROJECT_NAME}_lib)
set(TEST_NAME ${PROJECT_NAME}_test)

add_subdirectory(src)
add_subdirectory(test)
