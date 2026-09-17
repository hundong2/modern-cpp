if(NOT DEFINED EXECUTABLE OR NOT DEFINED INPUT_FILE OR NOT DEFINED EXPECTED_FILE)
  message(FATAL_ERROR "EXECUTABLE, INPUT_FILE, and EXPECTED_FILE are required")
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
  message(FATAL_ERROR "Exit ${exit_code}; stderr=[${stderr_text}]; stdout=[${actual}]")
endif()
file(READ "${EXPECTED_FILE}" expected)
string(REPLACE "\r\n" "\n" actual "${actual}")
string(REPLACE "\r\n" "\n" expected "${expected}")
string(REGEX REPLACE "\n$" "" actual "${actual}")
string(REGEX REPLACE "\n$" "" expected "${expected}")
if(NOT actual STREQUAL expected)
  message(FATAL_ERROR "expected=[${expected}], actual=[${actual}], stderr=[${stderr_text}]")
endif()
