# 준비한 입력을 제출 프로그램의 표준 입력으로 전달하고 출력을 수집한다.
execute_process(
  COMMAND "${EXECUTABLE}"
  INPUT_FILE "${INPUT_FILE}"
  OUTPUT_VARIABLE actual
  ERROR_VARIABLE stderr_text
  RESULT_VARIABLE exit_code)

# Windows CRLF와 Unix LF 차이가 정답 비교를 깨뜨리지 않도록 줄바꿈을 통일한다.
string(REPLACE "\r\n" "\n" actual "${actual}")
string(STRIP "${actual}" actual)
string(REPLACE "\r\n" "\n" expected "${EXPECTED}")
string(STRIP "${expected}" expected)

# 비정상 종료 또는 출력 불일치면 실제 출력과 표준 오류를 보여 주며 테스트를 실패시킨다.
if(NOT exit_code EQUAL 0 OR NOT actual STREQUAL expected)
  message(FATAL_ERROR "exit=${exit_code}\nexpected=[${expected}]\nactual=[${actual}]\nstderr=[${stderr_text}]")
endif()
