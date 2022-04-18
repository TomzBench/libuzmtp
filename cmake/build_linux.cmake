#------------------------------------------------------------------------------
# Add source
#------------------------------------------------------------------------------
set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/uzmtp_dealer.c
    ${CMAKE_SOURCE_DIR}/src/uzmtp_msg.c
)
set(HEADERS
    ${CMAKE_SOURCE_DIR}/src/uzmtp_dealer.h
    ${CMAKE_SOURCE_DIR}/src/uzmtp_msg.h
)
set(PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/uzmtp_types.h
    ${CMAKE_SOURCE_DIR}/include/uzmtp.h
	${CMAKE_SOURCE_DIR}/include/uzmtp_platform_linux.h
)

add_library(uzmtp ${SOURCES} ${HEADERS} ${PUBLIC_HEADERS})
target_compile_definitions(uzmtp PUBLIC "-DUZMTP_PLATFORM_HEADER=\"uzmtp_platform_linux.h\"")
target_include_directories(uzmtp PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(uzmtp PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

install(TARGETS uzmtp
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/uzmtp
)

if(UZMTP_ENABLE_TESTING)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        include(${CMAKE_MODULES_DIR}/CodeCoverage.cmake)
    endif()
    include("${CMAKE_EXTERNAL}/cmocka.cmake")
    enable_testing()
    include(CTest)

    add_executable(uzmtp-test
        ${CMAKE_SOURCE_DIR}/test/unit/main.c
        ${CMAKE_SOURCE_DIR}/test/unit/test_helpers.c
        ${CMAKE_SOURCE_DIR}/test/unit/test_helpers.h
        ${CMAKE_SOURCE_DIR}/test/unit/test_zmtp_dealer.c
        ${CMAKE_SOURCE_DIR}/test/unit/test_zmtp_msg.c
    )
    target_link_libraries(uzmtp-test uzmtp cmocka)
    install(TARGETS uzmtp-test DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
    add_test(NAME uzmtp-test COMMAND uzmtp-test)
endif()
