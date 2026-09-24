# 실행 파일, 입력 파일, 기대 출력 파일을 모두 받은 경우에만 비교를 수행한다.
if(NOT DEFINED EXECUTABLE OR NOT DEFINED INPUT_FILE OR NOT DEFINED EXPECTED_FILE)
  message(FATAL_ERROR "EXECUTABLE, INPUT_FILE, and EXPECTED_FILE are required")
endif()

# 프로그램에 파일 입력을 연결하고 종료 코드·stdout·stderr를 한 번에 수집한다.
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

# 플랫폼 개행을 LF로 통일하고 마지막 개행 하나는 답의 의미에서 제외한다.
file(READ "${EXPECTED_FILE}" expected)
string(REPLACE "\r\n" "\n" actual "${actual}")
string(REPLACE "\r\n" "\n" expected "${expected}")
string(REGEX REPLACE "\n$" "" actual "${actual}")
string(REGEX REPLACE "\n$" "" expected "${expected}")
if(NOT actual STREQUAL expected)
  message(FATAL_ERROR "expected=[${expected}], actual=[${actual}], stderr=[${stderr_text}]")
endif()
