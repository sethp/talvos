# Copyright (c) 2018 the Talvos developers. All rights reserved.
#
# This file is distributed under a three-clause BSD license. For full license
# terms please see the LICENSE file distributed with this source code.

# add_subdirectory(talvos-rt)
add_subdirectory(talvos)
if (NOT EMSCRIPTEN)
  add_subdirectory(vulkan)
endif()
