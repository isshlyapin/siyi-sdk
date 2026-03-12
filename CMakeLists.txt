cmake_minimum_required(VERSION 3.20)
project(siyi-sdk VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(siyi-sdk STATIC
  src/crc16.cxx
  src/protocol.cxx
  src/siyi_client.cxx
  src/siyi_sender.cxx
  src/udp_transport.cxx
  src/siyi_receiver.cxx
  src/siyi_client_impl.cxx
)
target_include_directories(siyi-sdk PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(siyi-sdk PRIVATE cxx_std_23)

add_executable(example1 example/example1.cxx)
target_link_libraries(example1 PRIVATE siyi-sdk)

add_executable(manual_control example/manual_control.cxx)
target_link_libraries(manual_control PRIVATE siyi-sdk)

#-----------------------------------------------------------------------------#
#                             Testing                                         #
#-----------------------------------------------------------------------------#
option(BUILD_TEST "Build tests" OFF)
if (BUILD_TEST)
    add_subdirectory(test)
endif()