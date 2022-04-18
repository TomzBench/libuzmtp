#------------------------------------------------------------------------------
# Add source
#------------------------------------------------------------------------------
set(SOURCES
    ${ZEPHYR_LIBUZMTP_MODULE_DIR}/src/uzmtp_dealer.c
    ${ZEPHYR_LIBUZMTP_MODULE_DIR}/src/uzmtp_msg.c
)

#------------------------------------------------------------------------------
# Add headers
#------------------------------------------------------------------------------
set(HEADERS
    ${ZEPHYR_LIBUZMTP_MODULE_DIR}/src/uzmtp_dealer.h
    ${ZEPHYR_LIBUZMTP_MODULE_DIR}/src/uzmtp_msg.h
)

#------------------------------------------------------------------------------
# Add public headers
#------------------------------------------------------------------------------
set(PUBLIC_HEADERS
    ${ZEPHYR_LIBUZMTP_MODULE_DIR}/include/uzmtp_types.h
    ${ZEPHYR_LIBUZMTP_MODULE_DIR}/include/uzmtp.h
    ${ZEPHYR_LIBUZMTP_MODULE_DIR}/include/uzmtp_platform.h
)

#------------------------------------------------------------------------------
# Generate uzmtp_platform.h for zephyr target
#------------------------------------------------------------------------------
configure_file(
    ${ZEPHYR_LIBUZMTP_MODULE_DIR}/include/uzmtp_platform_zephyr.h.in
    ${ZEPHYR_LIBUZMTP_MODULE_DIR}/include/uzmtp_platform.h
)

#------------------------------------------------------------------------------
# Add library
#------------------------------------------------------------------------------
zephyr_library()
zephyr_include_directories(${ZEPHYR_LIBUZMTP_MODULE_DIR}/include)
zephyr_library_sources(${SOURCES} ${HEADERS} ${PUBLIC_HEADERS})
zephyr_library_include_directories(${ZEPHYR_LIBUZMTP_MODULE_DIR}/include)
