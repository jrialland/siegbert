cmake_minimum_required(VERSION 3.10)

project(Siegbert)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

## Boost ##
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS regex)

################################################################################

file(GLOB_RECURSE SIEGBERT_EVALUATOR_SRC ${CMAKE_SOURCE_DIR}/src/evaluator/*.cpp)
file(GLOB_RECURSE SIEGBERT_GAME_SRC ${CMAKE_SOURCE_DIR}/src/game/*.cpp)
file(GLOB_RECURSE SIEGBERT_INTERFACE_SRC ${CMAKE_SOURCE_DIR}/src/interface/*.cpp)
file(GLOB_RECURSE SIEGBERT_PGN_SRC ${CMAKE_SOURCE_DIR}/src/pgn/*.cpp)
file(GLOB_RECURSE SIEGBERT_UNITTESTS_SRC ${CMAKE_SOURCE_DIR}/src/unittests/*.cpp)
set(SIEGBERT_MAIN_SRC ${CMAKE_SOURCE_DIR}/src/main.cpp)

################################################################################
add_executable(siegbert 
  ${SIEGBERT_EVALUATOR_SRC}
  ${SIEGBERT_GAME_SRC}
  ${SIEGBERT_INTERFACE_SRC}
  ${SIEGBERT_MAIN_SRC}
)

target_include_directories(siegbert PRIVATE 
    ${CMAKE_SOURCE_DIR}/deps/libpopcnt/
    ${CMAKE_SOURCE_DIR}/src/
)

target_link_libraries(siegbert ${Boost_LIBRARIES})

################################################################################
add_executable(unittests 
  ${SIEGBERT_UNITTESTS_SRC}
  ${SIEGBERT_EVALUATOR_SRC}
  ${SIEGBERT_PGN_SRC}
  ${SIEGBERT_GAME_SRC}
  ${SIEGBERT_INTERFACE_SRC}
)

target_compile_options(unittests
  PRIVATE
    -g
)

target_include_directories(unittests PRIVATE
    ${CMAKE_SOURCE_DIR}/deps/libpopcnt/
    ${CMAKE_SOURCE_DIR}/deps/catch2/
    ${CMAKE_SOURCE_DIR}/src/
)

target_link_libraries(unittests ${Boost_LIBRARIES})

enable_testing()
add_test(NAME UnitTests COMMAND ${PROJECT_BINARY_DIR}/unittests)
