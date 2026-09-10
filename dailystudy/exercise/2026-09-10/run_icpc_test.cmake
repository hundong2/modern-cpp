if(NOT DEFINED EXECUTABLE OR NOT DEFINED INPUT_FILE OR NOT DEFINED EXPECTED)
  message(FATAL_ERROR "EXECUTABLE, INPUT_FILE, EXPECTED are required")
endif()

execute_process(
  COMMAND "${EXECUTABLE}"
  INPUT_FILE "${INPUT_FILE}"
  OUTPUT_VARIABLE actual
  ERROR_VARIABLE stderr_text
  RESULT_VARIABLE exit_code)

if(NOT exit_code EQUAL 0)
  message(FATAL_ERROR "program exited with ${exit_code}: ${stderr_text}")
endif()

# Windows CRLF와 마지막 개행 하나만 정규화한다. 그 밖의 문자·공백 차이는 정확성 실패로 남긴다.
string(REPLACE "\r\n" "\n" actual "${actual}")
string(REGEX REPLACE "\n$" "" actual "${actual}")

if(NOT actual STREQUAL EXPECTED)
  message(FATAL_ERROR "expected=[${EXPECTED}] actual=[${actual}]")
endif()
