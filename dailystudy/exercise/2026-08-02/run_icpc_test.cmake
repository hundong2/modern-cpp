execute_process(
    COMMAND "${EXECUTABLE}"
    INPUT_FILE "${INPUT_FILE}"
    OUTPUT_VARIABLE actual
    RESULT_VARIABLE exit_code)
set(expected "0\n2\n3\n7\nINF\n")
if(NOT exit_code EQUAL 0)
    message(FATAL_ERROR "icpc_problem exit code: ${exit_code}")
endif()
if(NOT actual STREQUAL expected)
    message(FATAL_ERROR "unexpected output\nexpected=[${expected}]\nactual=[${actual}]")
endif()

