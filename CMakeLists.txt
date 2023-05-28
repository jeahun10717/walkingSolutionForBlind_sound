cmake_minimum_required(VERSION 3.5)
project(sound)

set(CMAKE_CXX_STANDARD 11)

add_executable(sound sound.cpp)

target_link_libraries(
  sound
  PRIVATE 
  asound
)
