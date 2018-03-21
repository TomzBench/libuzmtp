set(sources
	uzmtp_dealer.c
	uzmtp_msg.c
   )

set(headers
	uzmtp.h
	uzmtp_types.h
	uzmtp_dealer.h
	uzmtp_msg.h
   )

add_library(uzmtp ${sources} ${headers})
target_link_libraries(uzmtp)
target_include_directories(uzmtp PUBLIC .)
