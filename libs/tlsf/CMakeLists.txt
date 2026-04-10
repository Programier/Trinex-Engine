cmake_minimum_required(VERSION 3.5)
project(TLSF LANGUAGES CXX)

target_sources(TrinexEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/tlsf.cpp")
target_include_directories(TrinexEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
