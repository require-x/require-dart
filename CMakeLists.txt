cmake_minimum_required(VERSION 2.8)

project(requiredart)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(jxcore)

add_library(requiredart SHARED src/require.cc)

target_compile_definitions(requiredart PRIVATE DART_SHARED_LIB)
target_include_directories(requiredart PUBLIC /usr/lib/dart/include lib/src/include ${JXCORE_INCLUDE})

add_dependencies(requiredart jxcore)
target_link_libraries(requiredart ${JXCORE_LIBS})

install(TARGETS requiredart DESTINATION lib)
install(FILES require-dart.webpack.config.js DESTINATION etc)
