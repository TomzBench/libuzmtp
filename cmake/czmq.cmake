# czmq

ExternalProject_Add(czmq-project
	GIT_REPOSITORY https://github.com/zeromq/czmq
	UPDATE_COMMAND ""
	INSTALL_COMMAND make install
	LIST_SEPARATOR |
	CMAKE_ARGS 
		-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
	)
file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/include)

add_library(czmq STATIC IMPORTED)
add_dependencies(czmq-project zmq-project)
add_dependencies(czmq czmq-project)

set_property(TARGET czmq 
	PROPERTY IMPORTED_LOCATION 
	${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}czmq${CMAKE_STATIC_LIBRARY_SUFFIX}
	)
set_property(TARGET czmq 
	PROPERTY INTERFACE_INCLUDE_DIRECTORIES 
	${CMAKE_INSTALL_PREFIX}/include
	)
