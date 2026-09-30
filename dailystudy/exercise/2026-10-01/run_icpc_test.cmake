# 실행 파일, 입력 파일, 기대 출력 파일이 모두 전달되어야 정확한 출력 검사를 시작한다.
if(NOT DEFINED EXECUTABLE OR NOT DEFINED INPUT_FILE OR NOT DEFINED EXPECTED_FILE)
  message(FATAL_ERROR "EXECUTABLE, INPUT_FILE, and EXPECTED_FILE are required")
endif()

# 입력 파일을 stdin에 연결하고 종료 코드, stdout, stderr를 한 번에 수집한다.
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

# Windows CRLF와 Unix LF를 같은 의미로 비교하고 마지막 개행 하나만 무시한다.
file(READ "${EXPECTED_FILE}" expected)
string(REPLACE "\r\n" "\n" actual "${actual}")
string(REPLACE "\r\n" "\n" expected "${expected}")
string(REGEX REPLACE "\n$" "" actual "${actual}")
string(REGEX REPLACE "\n$" "" expected "${expected}")
if(NOT actual STREQUAL expected)
  message(FATAL_ERROR "expected=[${expected}], actual=[${actual}], stderr=[${stderr_text}]")
endif()
