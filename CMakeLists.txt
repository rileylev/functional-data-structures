cmake_minimum_required(VERSION 3.20)

project(
  functional_data_structures
  VERSION 0.0.0
  LANGUAGES CXX)

include(early_hook.cmake)

add_library(functional_data_structures INTERFACE)
add_library(functional_data_structures::functional_data_structures ALIAS functional_data_structures)
target_include_directories(functional_data_structures INTERFACE include)
target_compile_features(functional_data_structures INTERFACE cxx_std_20)

add_subdirectory(test)
