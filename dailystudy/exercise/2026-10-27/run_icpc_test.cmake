# 호출자가 실행 파일, stdin 파일, 기대 stdout 파일을 모두 전달했는지 먼저 검증한다.
if(NOT DEFINED EXECUTABLE OR NOT DEFINED INPUT_FILE OR NOT DEFINED EXPECTED_FILE)
  message(FATAL_ERROR "EXECUTABLE, INPUT_FILE, EXPECTED_FILE are required")
endif()

# 실제 프로그램을 주어진 입력으로 실행하고 stdout/stderr/종료 코드를 서로 다른 변수에 보존한다.
execute_process(
  COMMAND "${EXECUTABLE}"
  INPUT_FILE "${INPUT_FILE}"
  OUTPUT_VARIABLE actual
  ERROR_VARIABLE error_output
  RESULT_VARIABLE result
)

# 정상 종료하지 않은 경우 출력 비교로 실패 원인을 가리지 않고 즉시 stderr와 함께 보고한다.
if(NOT result EQUAL 0)
  message(FATAL_ERROR "program failed with code ${result}\nstderr=[${error_output}]")
endif()

# Windows CRLF와 Unix LF를 같은 논리 개행으로 정규화하고 마지막 개행 한 개는 비교에서 제외한다.
file(READ "${EXPECTED_FILE}" expected)
string(REPLACE "\r\n" "\n" actual "${actual}")
string(REPLACE "\r\n" "\n" expected "${expected}")
string(REGEX REPLACE "\n$" "" actual "${actual}")
string(REGEX REPLACE "\n$" "" expected "${expected}")

# 나머지 문자는 공백까지 정확히 같아야 하므로 정규식 부분 일치가 아니라 STREQUAL을 사용한다.
if(NOT actual STREQUAL expected)
  message(FATAL_ERROR "unexpected output\nexpected=[${expected}]\nactual=[${actual}]")
endif()
