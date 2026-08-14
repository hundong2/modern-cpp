# execute_process는 준비한 파일을 표준 입력으로 연결해 제출 프로그램을 실행한다.
execute_process(
  COMMAND "${EXECUTABLE}"
  INPUT_FILE "${INPUT_FILE}"
  OUTPUT_VARIABLE actual
  ERROR_VARIABLE error_output
  RESULT_VARIABLE result)

# Windows와 Unix 줄바꿈 차이를 없앤 뒤 끝 공백을 제거해 값만 비교한다.
string(REPLACE "\r\n" "\n" actual "${actual}")
string(STRIP "${actual}" actual)
string(REPLACE "\r\n" "\n" expected "${EXPECTED}")
string(STRIP "${expected}" expected)

# 프로그램 실패나 출력 불일치는 CTest 실패로 보고한다.
if(NOT result EQUAL 0)
  message(FATAL_ERROR "icpc_problem failed (${result}): ${error_output}")
endif()
if(NOT actual STREQUAL expected)
  message(FATAL_ERROR "expected=[${expected}] actual=[${actual}]")
endif()
