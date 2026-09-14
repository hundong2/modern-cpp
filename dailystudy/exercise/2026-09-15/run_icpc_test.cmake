if(NOT DEFINED EXECUTABLE OR NOT DEFINED INPUT_FILE OR NOT DEFINED EXPECTED_FILE)
  message(FATAL_ERROR "EXECUTABLE, INPUT_FILE, EXPECTED_FILE are required")
endif()

# 기대 출력도 파일에서 읽는다. -DEXPECTED=<문자열> 전달과 달리 여러 줄과 세미콜론이
# CMake 목록으로 다시 해석되지 않으므로 온라인 저지 출력을 손실 없이 비교할 수 있다.
file(READ "${EXPECTED_FILE}" expected)

# 실행 파일에 테스트 입력을 표준 입력으로 연결하고, 표준 출력·오류와 종료 코드를 각각 받는다.
# 10초 안에 끝나지 않는 프로그램은 무한 반복이나 지나치게 느린 구현으로 간주해 실패시킨다.
execute_process(
  COMMAND "${EXECUTABLE}"
  INPUT_FILE "${INPUT_FILE}"
  OUTPUT_VARIABLE actual
  ERROR_VARIABLE stderr_text
  RESULT_VARIABLE exit_code
  TIMEOUT 10)

# 정상 종료가 아니면 출력 비교보다 먼저 종료 코드와 표준 오류를 보여 주어 원인 진단을 돕는다.
if(NOT "${exit_code}" STREQUAL "0")
  message(FATAL_ERROR "program exited with ${exit_code}: ${stderr_text}")
endif()

# Windows CRLF와 관례적인 마지막 개행 하나만 양쪽에서 같은 방식으로 정규화한다.
# 그 밖의 공백, 빈 줄, 문자 차이는 숨기지 않고 정확한 출력 불일치로 남긴다.
string(REPLACE "\r\n" "\n" actual "${actual}")
string(REPLACE "\r\n" "\n" expected "${expected}")
string(REGEX REPLACE "\n$" "" actual "${actual}")
string(REGEX REPLACE "\n$" "" expected "${expected}")

if(NOT actual STREQUAL expected)
  message(FATAL_ERROR "expected=[${expected}] actual=[${actual}]")
endif()
