if(NOT DEFINED EXECUTABLE OR NOT DEFINED INPUT_FILE OR NOT DEFINED EXPECTED_FILE)
  message(FATAL_ERROR "EXECUTABLE, INPUT_FILE, EXPECTED_FILE are required")
endif()

# 기대 출력도 파일에서 읽는다. -DEXPECTED=<문자열> 방식과 달리 여러 줄이나 세미콜론이
# CMake 목록 구분자로 다시 해석되지 않으므로 임의의 judge 출력을 안전하게 비교할 수 있다.
file(READ "${EXPECTED_FILE}" expected)

execute_process(
  COMMAND "${EXECUTABLE}"
  INPUT_FILE "${INPUT_FILE}"
  OUTPUT_VARIABLE actual
  ERROR_VARIABLE stderr_text
  RESULT_VARIABLE exit_code
  TIMEOUT 10)

if(NOT "${exit_code}" STREQUAL "0")
  message(FATAL_ERROR "program exited with ${exit_code}: ${stderr_text}")
endif()

# Windows CRLF와 프로그램이 관례적으로 붙이는 마지막 개행 하나를 양쪽에서 같은 방식으로
# 정규화한다. 그 밖의 공백·빈 줄·문자 차이는 그대로 정확성 실패로 남긴다.
string(REPLACE "\r\n" "\n" actual "${actual}")
string(REPLACE "\r\n" "\n" expected "${expected}")
string(REGEX REPLACE "\n$" "" actual "${actual}")
string(REGEX REPLACE "\n$" "" expected "${expected}")

if(NOT actual STREQUAL expected)
  message(FATAL_ERROR "expected=[${expected}] actual=[${actual}]")
endif()
