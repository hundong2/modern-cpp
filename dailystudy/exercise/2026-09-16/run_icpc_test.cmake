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

# Windows 실행 파일의 CRLF와 저장된 LF를 같은 논리 줄바꿈으로 비교한다.
string(REPLACE "\r\n" "\n" actual "${actual}")
string(REPLACE "\r\n" "\n" expected "${expected}")

# 프로그램이 관례적으로 붙이는 마지막 개행 하나만 제거하고 내부 공백은 그대로 보존한다.
string(REGEX REPLACE "\n$" "" actual "${actual}")
string(REGEX REPLACE "\n$" "" expected "${expected}")

if(NOT actual STREQUAL expected)
  message(FATAL_ERROR
    "Output mismatch.\n"
    "expected=[${expected}]\n"
    "actual=[${actual}]\n"
    "stderr=[${stderr_text}]")
endif()
