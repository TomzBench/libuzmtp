#------------------------------------------------------------------------------
# Add source
#------------------------------------------------------------------------------
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uzmtp_dealer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uzmtp_msg.c
)

#------------------------------------------------------------------------------
# Add public headers
#------------------------------------------------------------------------------
set(PUBLIC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/uzmtp/uzmtp_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/uzmtp/uzmtp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/uzmtp/uzmtp_platform.h
)

#------------------------------------------------------------------------------
# Generate uzmtp_platform.h for linux target
#------------------------------------------------------------------------------
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/uzmtp/uzmtp_platform_linux.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/uzmtp/uzmtp_platform.h
)

#------------------------------------------------------------------------------
# Add library
#------------------------------------------------------------------------------
add_library(uzmtp ${SOURCES} ${PUBLIC_HEADERS})
target_include_directories(uzmtp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(uzmtp PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

install(TARGETS uzmtp
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/uzmtp
)

#------------------------------------------------------------------------------
# Build test binaries
#------------------------------------------------------------------------------
if(UZMTP_ENABLE_TESTING)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        include(${CMAKE_MODULES_DIR}/CodeCoverage.cmake)
    endif()
    include("${CMAKE_EXTERNAL}/cmocka.cmake")
    include("${CMAKE_EXTERNAL}/libzmq.cmake")
    enable_testing()
    include(CTest)

    # build test fixxture
    add_custom_command(
        OUTPUT "${FIXTURE_DIR}/node_modules"
        COMMAND npm install
        WORKING_DIRECTORY "${FIXTURE_DIR}")
    add_custom_target(node_modules DEPENDS "${FIXTURE_DIR}/node_modules")

    # Build dealer unit test exe
    add_executable(uzmtp-test-unit-dealer 
        ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/dealer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/helpers.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/helpers.h
    )
    target_link_libraries(uzmtp-test-unit-dealer uzmtp cmocka)
    install(TARGETS uzmtp-test-unit-dealer DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
    add_test(NAME uzmtp-test-unit-dealer COMMAND uzmtp-test-unit-dealer)

    # Build msg unit test exe
    add_executable(uzmtp-test-unit-msg
        ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/msg.c
    )
    target_link_libraries(uzmtp-test-unit-msg uzmtp cmocka)
    install(TARGETS uzmtp-test-unit-msg DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
    add_test(NAME uzmtp-test-unit-msg COMMAND uzmtp-test-unit-msg)


    # Build integration test exe
    add_executable(uzmtp-test-integration ${CMAKE_CURRENT_SOURCE_DIR}/test/integration/main.c)
    target_link_libraries(uzmtp-test-integration zmq-static uzmtp ${CMAKE_THREAD_LIBS_INIT} rt)
    set(TEST_COMMAND "node ${FIXTURE_DIR}/index.js ${CMAKE_BINARY_DIR}/uzmtp-test-integration")
    add_test(NAME uzmtp-test-integration 
        COMMAND "node"
        "${FIXTURE_DIR}/index.js"
        "${CMAKE_BINARY_DIR}/uzmtp-test-integration")
    add_dependencies(uzmtp-test-integration node_modules)
endif()
