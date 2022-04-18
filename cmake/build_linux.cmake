#------------------------------------------------------------------------------
# Add source
#------------------------------------------------------------------------------
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uzmtp_dealer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uzmtp_msg.c
)

#------------------------------------------------------------------------------
# Add headers
#------------------------------------------------------------------------------
set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uzmtp_dealer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uzmtp_msg.h
)

#------------------------------------------------------------------------------
# Add public headers
#------------------------------------------------------------------------------
set(PUBLIC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/uzmtp_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/uzmtp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/uzmtp_platform.h
)

#------------------------------------------------------------------------------
# Generate uzmtp_platform.h for linux target
#------------------------------------------------------------------------------
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/uzmtp_platform_linux.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/uzmtp_platform.h
)

#------------------------------------------------------------------------------
# Add library
#------------------------------------------------------------------------------
add_library(uzmtp ${SOURCES} ${HEADERS} ${PUBLIC_HEADERS})
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
    enable_testing()
    include(CTest)

    add_executable(uzmtp-test
        ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/main.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/test_helpers.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/test_helpers.h
        ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/test_zmtp_dealer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/test_zmtp_msg.c
    )
    target_link_libraries(uzmtp-test uzmtp cmocka)
    install(TARGETS uzmtp-test DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
    add_test(NAME uzmtp-test COMMAND uzmtp-test)
endif()
