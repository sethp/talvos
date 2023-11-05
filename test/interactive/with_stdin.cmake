# via https://stackoverflow.com/a/68617709
if (NOT EXISTS ${INPUT_FILE})
  message(FATAL_ERROR "Could not find file: `${INPUT_FILE}`")
endif()

set(ARG_NUM 1)
while(ARG_NUM LESS CMAKE_ARGC)
  if (${CMAKE_ARGV${ARG_NUM}} STREQUAL "--")
    break()
  endif()
  math(EXPR ARG_NUM "${ARG_NUM}+1")
  endwhile()

math(EXPR ARG_NUM "${ARG_NUM}+1")
set(COMMAND "")
while(ARG_NUM LESS CMAKE_ARGC)
  set(COMMAND ${COMMAND} ${CMAKE_ARGV${ARG_NUM}})
  math(EXPR ARG_NUM "${ARG_NUM}+1")
endwhile()

if(NOT COMMAND)
  # message(SEND_ERROR "missing command!")
  message(FATAL_ERROR "missing command!\n"
    "usage: ${CMAKE_ARGV0} [options] -- cmd")
endif()

execute_process(
  COMMAND ${COMMAND}
  COMMAND_ECHO STDERR
  INPUT_FILE "${INPUT_FILE}"
  COMMAND_ERROR_IS_FATAL ANY
)
