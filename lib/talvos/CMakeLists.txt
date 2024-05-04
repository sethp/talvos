# Copyright (c) 2018 the Talvos developers. All rights reserved.
#
# This file is distributed under a three-clause BSD license. For full license
# terms please see the LICENSE file distributed with this source code.

include(CheckIncludeFiles)
include(CheckLibraryExists)
include(FindThreads)

find_package(SPIRV-Headers REQUIRED)
find_package(SPIRV-Tools REQUIRED)

# TODO is there a convention for doing this? maybe `find_dependency`?
if (USE_INSTALLED_SPIRV-Tools)
unset(SPIRV-Tools_LIBRARIES)
find_library(SPIRV-Tools_LIBRARIES SPIRV-Tools)
endif()

if (NOT SPIRV-Tools_LIBRARIES)
	# message(STATUS "searching for SPIRV-Tools library in  ${SPIRV-Tools_LIBRARY_DIR}")
	find_library(SPIRV-Tools_LIBRARIES SPIRV-Tools PATHS "${SPIRV-Tools_LIBRARY_DIR}")
endif()
message(STATUS "SPIRV-Tools library: ${SPIRV-Tools_LIBRARIES}")


# Check for GNU readline library
if (NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  check_include_files("stdio.h;readline/readline.h" HAVE_READLINE_H)
  check_include_files("stdio.h;readline/history.h" HAVE_HISTORY_H)
  check_library_exists(readline readline
                       "${READLINE_DIR}/lib" HAVE_READLINE_LIB)
  check_library_exists(readline add_history
                       "${READLINE_DIR}/lib" HAVE_HISTORY_LIB)
  if (HAVE_READLINE_H AND HAVE_HISTORY_H AND
      HAVE_READLINE_LIB AND HAVE_HISTORY_LIB)
    set(HAVE_READLINE 1)
    list(APPEND TALVOS_OPTIONAL_LIBS readline)
  else()
    set(HAVE_READLINE 0)
    message(STATUS "GNU readline library not found. "
                    "The interactive debugger will not have a command history.")
  endif()
else()
  set(HAVE_READLINE 0)
endif()

# Disable exceptions for libtalvos
if (NOT MSVC)
  add_compile_options("-fno-exceptions")
endif()

# Generate config.h
configure_file("config.h.in" "config.h")

set(TALVOS_HEADERS
    ${PROJECT_SOURCE_DIR}/include/talvos/Block.h
    ${PROJECT_SOURCE_DIR}/include/talvos/Commands.h
    ${PROJECT_SOURCE_DIR}/include/talvos/ComputePipeline.h
    ${PROJECT_SOURCE_DIR}/include/talvos/Device.h
    ${PROJECT_SOURCE_DIR}/include/talvos/Dim3.h
    ${PROJECT_SOURCE_DIR}/include/talvos/EntryPoint.h
    ${PROJECT_SOURCE_DIR}/include/talvos/Function.h
    ${PROJECT_SOURCE_DIR}/include/talvos/GraphicsPipeline.h
    ${PROJECT_SOURCE_DIR}/include/talvos/Image.h
    ${PROJECT_SOURCE_DIR}/include/talvos/Instruction.h
    ${PROJECT_SOURCE_DIR}/include/talvos/Invocation.h
    ${PROJECT_SOURCE_DIR}/include/talvos/Memory.h
    ${PROJECT_SOURCE_DIR}/include/talvos/Module.h
    ${PROJECT_SOURCE_DIR}/include/talvos/Object.h
    ${PROJECT_SOURCE_DIR}/include/talvos/PipelineContext.h
    ${PROJECT_SOURCE_DIR}/include/talvos/PipelineStage.h
    ${PROJECT_SOURCE_DIR}/include/talvos/Plugin.h
    ${PROJECT_SOURCE_DIR}/include/talvos/Queue.h
    ${PROJECT_SOURCE_DIR}/include/talvos/RenderPass.h
    ${PROJECT_SOURCE_DIR}/include/talvos/Type.h
    ${PROJECT_SOURCE_DIR}/include/talvos/Variable.h
    ${PROJECT_SOURCE_DIR}/include/talvos/Workgroup.h)
set(TALVOS_SOURCES
    Block.cpp
    Commands.cpp
    ComputePipeline.cpp
    Device.cpp
    Dim3.cpp
    Function.cpp
    GraphicsPipeline.cpp
    Image.cpp
    Instruction.cpp
    Invocation.cpp
    Memory.cpp
    Module.cpp
    Object.cpp
    PipelineContext.cpp
    PipelineExecutor.cpp
    PipelineStage.cpp
    Queue.cpp
    RenderPass.cpp
    Type.cpp
    Variable.cpp
    Workgroup.cpp
    Utils.cpp
    Utils.h)

if (NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" AND NOT EMSCRIPTEN)
  set(CORE_LIB_TYPE SHARED)
endif()

add_library(talvos ${CORE_LIB_TYPE} ${TALVOS_SOURCES} ${TALVOS_HEADERS})
target_include_directories(talvos PUBLIC "${SPIRV-Headers_INCLUDE_DIRS}")
target_include_directories(talvos PUBLIC "${SPIRV-Tools_INCLUDE_DIRS}")
target_include_directories(talvos PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(talvos "${SPIRV-Tools_LIBRARIES}")
target_link_libraries(talvos "${CMAKE_DL_LIBS}")
target_link_libraries(talvos "${CMAKE_THREAD_LIBS_INIT}")
target_link_libraries(talvos "${TALVOS_OPTIONAL_LIBS}")

install(TARGETS talvos DESTINATION lib)
install(FILES ${TALVOS_HEADERS} DESTINATION include/talvos)
