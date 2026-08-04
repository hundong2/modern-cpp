execute_process(
    COMMAND "${EXECUTABLE}"
    INPUT_FILE "${INPUT_FILE}"
    OUTPUT_VARIABLE actual
    RESULT_VARIABLE result)
string(STRIP "${actual}" actual)
if(NOT result EQUAL 0 OR NOT actual STREQUAL EXPECTED)
    message(FATAL_ERROR "expected '${EXPECTED}', got '${actual}', exit=${result}")
endif()
