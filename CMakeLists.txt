cmake_minimum_required(VERSION 3.5.1)

project(embedded_nn_loader)

add_subdirectory(jsmn_util)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/jsmn_util/jsmn)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/jsmn_util/inc)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/inc)

add_definitions(-DJSMN_PARENT_LINKS)
add_compile_options(-g3)

add_library(embedded_nn_loader src/nn_order.c)
add_executable(embedded_nn_loader_example example/reorder/example_reorder.c)
add_dependencies(embedded_nn_loader_example embedded_nn_loader jsmn_util)
target_link_libraries(embedded_nn_loader_example embedded_nn_loader jsmn_util libjsmn)
