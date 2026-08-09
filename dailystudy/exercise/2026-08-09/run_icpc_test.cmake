execute_process(COMMAND "${EXECUTABLE}" INPUT_FILE "${INPUT_FILE}"
    OUTPUT_VARIABLE actual RESULT_VARIABLE result OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "프로그램 종료 코드: ${result}")
endif()
if(NOT actual STREQUAL EXPECTED)
    message(FATAL_ERROR "기대값 '${EXPECTED}', 실제값 '${actual}'")
endif()
