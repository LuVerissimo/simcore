cmake_minimum_required(VERSION 3.25)
project(simcore VERSION 1.0  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_CUDA_HOST_COMPILER)
  set(CMAKE_CUDA_HOST_COMPILER ${CMAKE_CXX_COMPILER})
endif()

# Default to Debug (sanitizers on) unless told otherwise.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# --- The non-negotiable flags ---

add_compile_options(
  $<$<COMPILE_LANGUAGE:CXX>:-Wall>
  $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
  $<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(
    $<$<COMPILE_LANGUAGE:CXX>:-fsanitize=address,undefined>
    $<$<COMPILE_LANGUAGE:CXX>:-g>
    $<$<COMPILE_LANGUAGE:CXX>:-O0>
  )
  add_link_options(-fsanitize=address,undefined)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_compile_options(
    $<$<COMPILE_LANGUAGE:CXX>:-O3>
    $<$<COMPILE_LANGUAGE:CXX>:-march=native>
  )
endif()

option(SIMCORE_CUDA "Enable CUDA acceleration and processing modules" OFF)
if(SIMCORE_CUDA)
  include(CheckLanguage)
  check_language(CUDA)
  
  if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
    set(CMAKE_CUDA_STANDARD 17)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
    set(CMAKE_CUDA_EXTENSIONS OFF)
    message(STATUS "CUDA Toolkit detected: ${CMAKE_CUDA_COMPILER}")
  else()
    message(FATAL_ERROR "CUDA language support requested but toolkit was not found")
  endif()
endif()

# --- Header-only math library ---
add_library(simcore_math INTERFACE)
target_include_directories(simcore_math INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_library(sys_utils INTERFACE)
target_include_directories(sys_utils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(Utils::sys_utils ALIAS sys_utils)

# --- Test framework: doctest, fetched automatically ---
include(FetchContent)
# doctest v2.4.11 declares an old cmake_minimum_required; this keeps CMake 4.x happy
# (ignored by older CMake versions).
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG        v2.4.11
)
FetchContent_MakeAvailable(doctest)

enable_testing()
add_subdirectory(tests)


if(SIMCORE_CUDA)
  add_library(simcore_cuda src/cuda/saxpy.cu src/cuda/vec3.cu)
  target_include_directories(simcore_cuda PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_link_libraries(simcore_cuda simcore_math)

  add_executable(cuda_test_device_buffer tests/cuda/test_device_buffer.cu)
  target_link_libraries(cuda_test_device_buffer simcore_cuda)

  add_executable(cuda_test_saxpy tests/cuda/test_saxpy.cu)
  target_link_libraries(cuda_test_saxpy simcore_cuda)

  add_executable(cuda_test_vec3 tests/cuda/test_vec3.cu)
  target_link_libraries(cuda_test_vec3 simcore_cuda)

  add_executable(bench_saxpy tests/cuda/bench_saxpy.cu)
  target_link_libraries(bench_saxpy simcore_cuda)
endif()