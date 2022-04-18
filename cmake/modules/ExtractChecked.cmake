function (extract_checked dl test_file)
  get_filename_component(REPO "${test_file}" DIRECTORY)
  if(NOT EXISTS "${test_file}")
    message(STATUS "${test_file} not found! extracting ${REPO}")
    execute_process(
    	COMMAND tar -xzvf "${dl}" -C "${EXTERNAL_DIR}"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    	RESULT_VARIABLE EXTRACT_RESULT)
    message(STATUS "EXTRACT_RESULT: ${EXTRACT_RESULT}")
  else()
    message(STATUS "${test_file} found!")
  endif()
endfunction()
