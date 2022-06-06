#------------------------------------------------------------------------------
# Add source
#------------------------------------------------------------------------------
set(SOURCES
    ${ZEPHYR_LIBUZMTP_MODULE_DIR}/src/dealer.c
    ${ZEPHYR_LIBUZMTP_MODULE_DIR}/src/msg.c
)

#------------------------------------------------------------------------------
# Add public headers
#------------------------------------------------------------------------------
set(PUBLIC_HEADERS
    ${ZEPHYR_LIBUZMTP_MODULE_DIR}/include/uzmtp/uzmtp_types.h
    ${ZEPHYR_LIBUZMTP_MODULE_DIR}/include/uzmtp/uzmtp.h
    ${ZEPHYR_LIBUZMTP_MODULE_DIR}/include/uzmtp/uzmtp_platform.h
)

#------------------------------------------------------------------------------
# Generate uzmtp_platform.h for zephyr target
#------------------------------------------------------------------------------
configure_file(
    ${ZEPHYR_LIBUZMTP_MODULE_DIR}/include/uzmtp/uzmtp_platform_zephyr.h.in
    ${ZEPHYR_LIBUZMTP_MODULE_DIR}/include/uzmtp/uzmtp_platform.h
)

#------------------------------------------------------------------------------
# Add library
#------------------------------------------------------------------------------
zephyr_library()
zephyr_include_directories(${ZEPHYR_LIBUZMTP_MODULE_DIR}/include)
zephyr_library_sources(${SOURCES} ${PUBLIC_HEADERS})
zephyr_library_include_directories(${ZEPHYR_LIBUZMTP_MODULE_DIR}/include)
