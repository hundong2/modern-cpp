execute_process(COMMAND "${EXECUTABLE}" INPUT_FILE "${INPUT_FILE}"
    OUTPUT_VARIABLE actual RESULT_VARIABLE exit_code)
if(NOT exit_code EQUAL 0)
    message(FATAL_ERROR "icpc_problem exit code: ${exit_code}")
endif()
set(expected "0\n2\n4\nImpossible\n\n")
if(NOT actual STREQUAL expected)
    message(FATAL_ERROR "unexpected output\nexpected=[${expected}]\nactual=[${actual}]")
endif()
