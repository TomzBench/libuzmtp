#------------------------------------------------------------------------------
# Build the app-zmtp
#------------------------------------------------------------------------------

find_package(Threads)
add_executable(app-zmtp echo-fixture.c main.c)
target_link_libraries(app-zmtp czmq zmq uzmtp ${CMAKE_THREAD_LIBS_INIT} rt)
install(TARGETS app-zmtp DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
