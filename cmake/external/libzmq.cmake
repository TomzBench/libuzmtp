# libzmq

set(ZMQ_VERSION "4.3.4")
set(ZMQ_SOURCE_DIR "${EXTERNAL_DIR}/zeromq-${ZMQ_VERSION}")
set(ZMQ_TEST_FILE "${ZMQ_SOURCE_DIR}/CMakeLists.txt")
set(zmq_VERSION "4_3_4")
extract_checked("${DOWNLOAD_DIR}/zeromq-${ZMQ_VERSION}.tar.gz" "${ZMQ_TEST_FILE}")

if(MSVC)
  # NOTE - we use CMAKE 3.15 policy for setting MSVC Runtime Library (Which 
  # leaves absent the /MT switch from cache FLAG variables in favor of the 
  # CMAKE_MSVC_RUNTIME_LIBRARY variable.  The ZMQ CMakeLists does not support
  # The MSVC Runtime variable so we explicitly add the switch here.
  set(ZMQ_C_FLAGS "${CMAKE_C_FLAGS} /MT")
  set(ZMQ_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")
  set(ZMQ_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
  set(ZMQ_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
else()
  set(ZMQ_C_FLAGS "${CMAKE_C_FLAGS}")
  set(ZMQ_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(ZMQ_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
  set(ZMQ_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endif()

ExternalProject_Add(zmq-project
	SOURCE_DIR ${ZMQ_SOURCE_DIR}
	INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
	UPDATE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND
		cmake
		--build .
		--target install
		--config Release
	LIST_SEPARATOR |
	CMAKE_ARGS 
		-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
		-DCMAKE_INSTALL_LIBDIR=<INSTALL_DIR>/lib
		-DCMAKE_C_FLAGS:STRING=${ZMQ_C_FLAGS}
		-DCMAKE_C_FLAGS_RELEASE:STRING=${ZMQ_C_FLAGS_RELEASE}
		-DCMAKE_CXX_FLAGS:STRING=${ZMQ_CXX_FLAGS}
		-DCMAKE_CXX_FLAGS_RELEASE:STRING=${ZMQ_CXX_FLAGS_RELEASE}
		-DZMQ_BUILD_TESTS:BOOL=OFF 
		-DENABLE_WS:BOOL=OFF
		-DENABLE_CURVE:BOOL=OFF
		-DBUILD_TESTS:BOOL=OFF 
		-DBUILD_STATIC:BOOL=ON
		-DBUILD_SHARED:BOOL=OFF
		-DWITH_DOCS:BOOL=OFF
		-DWITH_PERF_TOOL:BOOL=OFF
	)

ExternalProject_Get_Property(zmq-project install_dir)
set(zmq_INCLUDE_DIR ${install_dir}/include)
FILE(MAKE_DIRECTORY ${install_dir}/include)
IF(NOT MSVC)
  set(zmq_static_LIBRARY ${CMAKE_STATIC_LIBRARY_PREFIX}zmq${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(zmq_shared_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}zmq${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(zmq_static_LIBRARY_LOC ${install_dir}/lib/${zmq_static_LIBRARY})
  set(zmq_shared_LIBRARY_LOC ${install_dir}/lib/${zmq_shared_LIBRARY})
ELSE()
  MESSAGE(STATUS "zmq_VERSION: ${zmq_VERSION}")
  set(zmq_static_LIBRARY libzmq-${CMAKE_VS_PLATFORM_TOOLSET}-mt-s-${zmq_VERSION}${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(zmq_shared_LIBRARY libzmq-${CMAKE_VS_PLATFORM_TOOLSET}-mt-${zmq_VERSION}${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(zmq_static_LIBRARY_LOC ${install_dir}/lib/${zmq_static_LIBRARY})
  set(zmq_shared_LIBRARY_LOC ${install_dir}/bin/${zmq_shared_LIBRARY})
ENDIF()

# zmq-static
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/libzmq-static-loc.txt ${zmq_static_LIBRARY})
add_library(zmq-static STATIC IMPORTED)
set_property(TARGET zmq-static PROPERTY IMPORTED_LOCATION ${zmq_static_LIBRARY_LOC})
set_property(TARGET zmq-static PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${zmq_INCLUDE_DIR})
add_dependencies(zmq-static zmq-project)

# zmq-shared
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/libzmq-shared-loc.txt ${zmq_shared_LIBRARY})
add_library(zmq-shared STATIC IMPORTED)
set_property(TARGET zmq-shared PROPERTY IMPORTED_LOCATION ${zmq_shared_LIBRARY_LOC})
set_property(TARGET zmq-shared PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${zmq_INCLUDE_DIR})
add_dependencies(zmq-shared zmq-project)
