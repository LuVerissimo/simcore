cmake_minimum_required(VERSION 3.25)
project(simcore CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Debug (sanitizers on) unless told otherwise.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# --- The non-negotiable flags ---
add_compile_options(-Wall -Wextra -Wpedantic)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-fsanitize=address,undefined -g -O0)
  add_link_options(-fsanitize=address,undefined)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_compile_options(-O3 -march=native)
endif()

# --- Header-only math library ---
add_library(simcore_math INTERFACE)
target_include_directories(simcore_math INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

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
