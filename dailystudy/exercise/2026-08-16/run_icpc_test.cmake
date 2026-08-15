# CMake 스크립트 모드는 준비된 입력을 제출 프로그램에 전달하고 출력을 비교한다.
execute_process(
  COMMAND "${EXECUTABLE}"
  INPUT_FILE "${INPUT_FILE}"
  OUTPUT_VARIABLE actual
  ERROR_VARIABLE stderr_text
  RESULT_VARIABLE exit_code)

# Windows와 Unix의 줄 끝 차이가 정답 비교를 깨뜨리지 않도록 CR 문자를 제거한다.
string(REPLACE "\r\n" "\n" actual "${actual}")
# 답 마지막 개행 유무는 채점 의미가 없으므로 양끝 공백을 정규화한다.
string(STRIP "${actual}" actual)
# -D 인자로 받은 기대 출력도 같은 방식으로 정규화한다.
string(REPLACE "\r\n" "\n" expected "${EXPECTED}")
string(STRIP "${expected}" expected)

# 비정상 종료나 다른 출력이면 입력별 회귀 테스트를 즉시 실패시킨다.
if(NOT exit_code EQUAL 0 OR NOT actual STREQUAL expected)
  message(FATAL_ERROR "exit=${exit_code}\nexpected=[${expected}]\nactual=[${actual}]\nstderr=[${stderr_text}]")
endif()
