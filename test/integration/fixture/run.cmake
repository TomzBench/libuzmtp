message(STATUS "CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")
execute_process(COMMAND bash -c "node index.js &" 
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	INPUT_FILE /dev/null
	OUTPUT_FILE /dev/null
	ERROR_FILE /dev/null)
