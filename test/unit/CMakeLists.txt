#------------------------------------------------------------------------------
# Build the zmtp-test app
#------------------------------------------------------------------------------

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMOCKA_PUBLIC_INCLUDE_DIRS}
)

add_executable(zmtp-test main.c test_zmtp_msg.c test_zmtp_dealer.c test_helpers.c)

target_link_libraries(zmtp-test uzmtp cmocka)
install(TARGETS zmtp-test DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_test(NAME zmtp-test COMMAND zmtp-test)
