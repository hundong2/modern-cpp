# 실행 파일과 입력·기대 출력 파일을 모두 전달받아야 정확 출력 비교를 수행할 수 있다.
if(NOT DEFINED EXECUTABLE OR NOT DEFINED INPUT_FILE OR NOT DEFINED EXPECTED_FILE)
  message(FATAL_ERROR "EXECUTABLE, INPUT_FILE, and EXPECTED_FILE are required")
endif()

# 테스트 입력을 표준 입력으로 연결하고 종료 상태, 표준 출력, 표준 오류를 함께 수집한다.
execute_process(
  COMMAND "${EXECUTABLE}"
  INPUT_FILE "${INPUT_FILE}"
  OUTPUT_VARIABLE actual
  ERROR_VARIABLE stderr_text
  RESULT_VARIABLE exit_code
  TIMEOUT 20
)

# 정상 종료하지 않은 프로그램은 출력이 우연히 같더라도 실패로 처리한다.
if(NOT "${exit_code}" STREQUAL "0")
  message(FATAL_ERROR "Exit ${exit_code}; stderr=[${stderr_text}]; stdout=[${actual}]")
endif()

# Windows CRLF와 Unix LF를 같은 줄바꿈으로 정규화하고 마지막 개행 하나만 무시한다.
file(READ "${EXPECTED_FILE}" expected)
string(REPLACE "\r\n" "\n" actual "${actual}")
string(REPLACE "\r\n" "\n" expected "${expected}")
string(REPLACE "\r" "\n" actual "${actual}")
string(REPLACE "\r" "\n" expected "${expected}")
string(REGEX REPLACE "\n$" "" actual "${actual}")
string(REGEX REPLACE "\n$" "" expected "${expected}")

if(NOT actual STREQUAL expected)
  message(FATAL_ERROR "expected=[${expected}], actual=[${actual}], stderr=[${stderr_text}]")
endif()
