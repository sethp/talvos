# Copyright (c) 2018 the Talvos developers. All rights reserved.
#
# This file is distributed under a three-clause BSD license. For full license
# terms please see the LICENSE file distributed with this source code.

foreach(test
  break
  break-on-error
  print
  step
  switch
)
  set(TEST_NAME "interactive/${test}")
  # via https://stackoverflow.com/a/68617709
  add_test(
    NAME ${TEST_NAME}
    COMMAND ${CMAKE_COMMAND}
    -D INPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/${test}.stdin
    -P ${CMAKE_CURRENT_SOURCE_DIR}/with_stdin.cmake
    --
    ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/test/run-test.py
    ${TEST_WRAPPER} $<TARGET_FILE:talvos-cmd>
    ${CMAKE_CURRENT_SOURCE_DIR}/${test}.tcf
  )

  # Enable interactive mode.
  set_tests_properties(${TEST_NAME} PROPERTIES
                       ENVIRONMENT "TALVOS_INTERACTIVE=1")
endforeach(${test})
