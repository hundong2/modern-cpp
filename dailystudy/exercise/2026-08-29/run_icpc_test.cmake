# CTest가 전달한 실행 파일·입력 파일·기대값이 모두 있어야 비교할 수 있다.
if(NOT DEFINED EXECUTABLE OR NOT DEFINED INPUT_FILE OR NOT DEFINED EXPECTED)
  message(FATAL_ERROR "EXECUTABLE, INPUT_FILE, and EXPECTED are required")
endif()

# INPUT_FILE을 표준 입력으로 연결해 제출 프로그램을 실행하고 출력과 종료 코드를 받는다.
execute_process(
  COMMAND "${EXECUTABLE}"
  INPUT_FILE "${INPUT_FILE}"
  OUTPUT_VARIABLE actual
  ERROR_VARIABLE error_output
  RESULT_VARIABLE result
)

if(NOT result EQUAL 0)
  message(FATAL_ERROR "program exited with ${result}: ${error_output}")
endif()

# 줄 끝 차이만 제거하고 실제 정답 문자열은 그대로 비교한다.
string(STRIP "${actual}" actual)
string(STRIP "${EXPECTED}" expected)
if(NOT actual STREQUAL expected)
  message(FATAL_ERROR "expected '${expected}', got '${actual}'")
endif()
