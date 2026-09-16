if(NOT DEFINED EXECUTABLE)
  message(FATAL_ERROR "EXECUTABLE is required")
endif()
if(NOT DEFINED INPUT_FILE)
  message(FATAL_ERROR "INPUT_FILE is required")
endif()
if(NOT DEFINED EXPECTED_FILE)
  message(FATAL_ERROR "EXPECTED_FILE is required")
endif()

execute_process(
  COMMAND "${EXECUTABLE}"
  INPUT_FILE "${INPUT_FILE}"
  OUTPUT_VARIABLE actual
  ERROR_VARIABLE stderr_text
  RESULT_VARIABLE exit_code
  TIMEOUT 10
)

if(NOT "${exit_code}" STREQUAL "0")
  message(FATAL_ERROR
    "Program exited with ${exit_code}.\n"
    "stderr:\n${stderr_text}\n"
    "stdout:\n${actual}")
endif()

file(READ "${EXPECTED_FILE}" expected)
string(REPLACE "\r\n" "\n" actual "${actual}")
string(REPLACE "\r\n" "\n" expected "${expected}")
string(REGEX REPLACE "\n$" "" actual "${actual}")
string(REGEX REPLACE "\n$" "" expected "${expected}")

if(NOT actual STREQUAL expected)
  message(FATAL_ERROR
    "Output mismatch.\nexpected=[${expected}]\nactual=[${actual}]\nstderr=[${stderr_text}]")
endif()
