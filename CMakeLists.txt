# Copyright (c) 2018 the Talvos developers. All rights reserved.
#
# This file is distributed under a three-clause BSD license. For full license
# terms please see the LICENSE file distributed with this source code.

cmake_minimum_required(VERSION 3.8.2)
project(Talvos)

if (EMSCRIPTEN)
  add_compile_options(
    # -sSHARED_MEMORY
    -sSTRICT # see: https://github.com/emscripten-core/emscripten/blob/5b24a09de582f3943d75aa6af9be36ba719c08c0/src/settings.js#L1110-L1126
  )

  # add_link_options(-sSHARED_MEMORY)
  # add_link_options(-D__EMSCRIPTEN_SHARED_MEMORY__=1 -matomics -mbulk-memory)

  add_compile_options(-fwasm-exceptions -O0) # -Og breaks wasm exceptions (?!)
  add_link_options(-fwasm-exceptions -Og)

  add_compile_options(-g3 -gdwarf-5 -gsplit-dwarf -gpubnames)
  # add_compile_options(-gembed-source) # TODO the goggles, they do nothing
  add_link_options(-g3 -gdwarf-5)

  # FRAGILE: these only work when symlinks exist at /@fs/... for `llvm-dwp`
  # FRAGILE: depends on the location of the binary in the server lol
  # (see: Dockerfile.emscripten)
  # TODO[seth]: it looks like another possible option here is to use an absolute path? which the chrome dev tools "just" resolve with `file://`
  # add_compile_options(-ffile-prefix-map=/usr/src/SPIRV-Tools=../@fs/home/seth/Code/src/github.com/sethp/learn-gpgpu/tmp/_SPIRV-Tools)
  # add_compile_options(-ffile-prefix-map=/usr/src/SPIRV-Headers=../@fs/home/seth/Code/src/github.com/sethp/learn-gpgpu/tmp/_SPIRV-Headers)

  # add_compile_options(-fdebug-compilation-dir=.) # TODO w.. t... f.....
  # add_compile_options(-ffile-prefix-map=/usr/src/talvos=../wasm/talvos)

  ## a monument to the ideas that didn't work:

  # add_compile_options(-ffile-prefix-map=/home/seth/.emscripten_cache/sysroot/=./foo/bar )
  # add_compile_options(-ffile-prefix-map=/home/seth/.emscripten_cache/sysroot/=./foo/bar )
  # add_compile_options(-ffile-prefix-map=../../=./foo/bar )

  # add_compile_options(-ffile-prefix-map=/=/@fs/)


  # add_link_options(-fdebug-prefix-map=/emsdk/upstream/emscripten/cache/sysroot=/@fs/home/seth/.emscripten_cache/sysroot)
  # add_link_options(-fdebug-prefix-map=/=/@fs/)
  # add_compile_options(-fdebug-prefix-map=.=/@fs/home/seth/Code/src/github.com/talvos/talvos )
  # add_compile_options(-fdebug-prefix-map=/=/@fs/)
  # add_compile_options(-fdebug-prefix-map=/=/@fs/ -fdebug-prefix-map=.=/@fs/home/seth/Code/src/github.com/talvos/talvos )
  # add_link_options(-g3)
endif()

set(TALVOS_VERSION_MAJOR 0)
set(TALVOS_VERSION_MINOR 1)
set(TALVOS_VERSION_PATCH 0)
set(TALVOS_VERSION_SUFFIX "-dev")
configure_file("version.h.in" "version.h")
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include(CheckIncludeFileCXX)

# Enable rpath on OS X
set(CMAKE_MACOSX_RPATH 1)

# Require and enable C++17 support, and disable compiler extensions.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Disable non-secure function warnings for MSVC
if (MSVC)
  add_definitions("-D_CRT_SECURE_NO_WARNINGS")
endif()

include_directories(include)

add_subdirectory(docs)
add_subdirectory(lib)
add_subdirectory(tools)

# Check for Python
find_package(Python3)
if (Python3_FOUND)
  enable_testing()
  add_subdirectory(test)
else()
  message(WARNING "Skipping tests (Python 3 required)")
endif()
