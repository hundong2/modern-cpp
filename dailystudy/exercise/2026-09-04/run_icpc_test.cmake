if(NOT DEFINED EXECUTABLE OR NOT DEFINED INPUT_FILE OR NOT DEFINED EXPECTED)
  message(FATAL_ERROR "EXECUTABLE, INPUT_FILE, EXPECTED are required")
endif()

execute_process(
  COMMAND "${EXECUTABLE}"
  INPUT_FILE "${INPUT_FILE}"
  OUTPUT_VARIABLE actual
  ERROR_VARIABLE stderr_output
  RESULT_VARIABLE result)

if(NOT result EQUAL 0)
  message(FATAL_ERROR "program exited with ${result}: ${stderr_output}")
endif()

string(REPLACE "\r\n" "\n" actual "${actual}")
string(REGEX REPLACE "\n$" "" actual "${actual}")
if(NOT actual STREQUAL EXPECTED)
  message(FATAL_ERROR "expected=[${EXPECTED}] actual=[${actual}]")
endif()
