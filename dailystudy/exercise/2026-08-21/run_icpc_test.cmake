# 준비한 입력 파일을 실행 파일의 표준 입력으로 전달하고 출력과 종료 코드를 수집한다.
execute_process(
  COMMAND "${EXECUTABLE}"
  INPUT_FILE "${INPUT_FILE}"
  OUTPUT_VARIABLE actual
  ERROR_VARIABLE stderr_text
  RESULT_VARIABLE exit_code)

# Windows CRLF와 Unix LF 차이를 없애고 양 끝 공백은 정답 비교에서 제외한다.
string(REPLACE "\r\n" "\n" actual "${actual}")
string(STRIP "${actual}" actual)
string(REPLACE "\r\n" "\n" expected "${EXPECTED}")
string(STRIP "${expected}" expected)

# 비정상 종료 또는 출력 불일치면 실제 결과를 보여 주며 CTest를 실패시킨다.
if(NOT exit_code EQUAL 0 OR NOT actual STREQUAL expected)
  message(FATAL_ERROR "exit=${exit_code}\nexpected=[${expected}]\nactual=[${actual}]\nstderr=[${stderr_text}]")
endif()
