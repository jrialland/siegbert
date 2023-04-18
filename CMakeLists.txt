cmake_minimum_required(VERSION 3.10)

project(Siegbert)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_ENABLE_EXPORTS TRUE)
SET (CMAKE_CXX_FLAGS_DEBUG_INIT "-ggdb")
SET (CMAKE_CXX_FLAGS_RELEASE_INIT "-O3 -DNDEBUG")

## pthread ##
find_package(Threads REQUIRED)

################################################################################

file(GLOB_RECURSE SIEGBERT_UTILS_SRC ${CMAKE_SOURCE_DIR}/src/utils/*.cpp)
file(GLOB_RECURSE SIEGBERT_EVALUATOR_SRC ${CMAKE_SOURCE_DIR}/src/evaluator/*.cpp)
file(GLOB_RECURSE SIEGBERT_GAME_SRC ${CMAKE_SOURCE_DIR}/src/game/*.cpp)
file(GLOB_RECURSE SIEGBERT_INTERFACE_SRC ${CMAKE_SOURCE_DIR}/src/interface/*.cpp)
file(GLOB_RECURSE SIEGBERT_LOGGING_SRC ${CMAKE_SOURCE_DIR}/src/logging/*.cpp)

file(GLOB_RECURSE SIEGBERT_OPENINGBOOK_SRC ${CMAKE_SOURCE_DIR}/src/openingbook/*.cpp)
file(GLOB_RECURSE SIEGBERT_PGN_SRC ${CMAKE_SOURCE_DIR}/src/pgn/*.cpp)
file(GLOB_RECURSE SIEGBERT_THREADING_SRC ${CMAKE_SOURCE_DIR}/src/threading/*.cpp)

set(SIEGBERT_SRC
  ${SIEGBERT_UTILS_SRC}
  ${SIEGBERT_EVALUATOR_SRC}
  ${SIEGBERT_GAME_SRC}
  ${SIEGBERT_INTERFACE_SRC}
  ${SIEGBERT_LOGGING_SRC}
  ${SIEGBERT_OPENINGBOOK_SRC}
  ${SIEGBERT_PGN_SRC}
  ${SIEGBERT_THREADING_SRC}
)

file(GLOB_RECURSE SIEGBERT_UNITTESTS_SRC ${CMAKE_SOURCE_DIR}/src/unittests/*.cpp)

set(SIEGBERT_MAIN ${CMAKE_SOURCE_DIR}/src/main.cpp)

################################################################################
add_executable(siegbert 
  ${SIEGBERT_SRC}
  ${SIEGBERT_MAIN}
)

target_include_directories(siegbert PRIVATE 
    ${CMAKE_SOURCE_DIR}/deps/libpopcnt/
    ${CMAKE_SOURCE_DIR}/src/
)

target_link_libraries(siegbert ${CMAKE_DL_LIBS})
target_link_libraries(siegbert ${CMAKE_THREAD_LIBS_INIT} )
target_link_libraries(siegbert ${Boost_LIBRARIES})

################################################################################
add_executable(unittests 
  ${SIEGBERT_SRC}
  ${SIEGBERT_UNITTESTS_SRC}
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

target_link_libraries(unittests ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})


enable_testing()
add_test(NAME UnitTests COMMAND ${PROJECT_BINARY_DIR}/unittests)
