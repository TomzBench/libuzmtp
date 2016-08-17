# uzmtp.mk

CFLAGS?=-O0 -g -Wall -DNDEBUG -DUZMTP_USE_LINUX -DUZMTP_USE_WOLFSSL
LDFLAGS:=
CC:=cc -std=gnu11 -I ./src/port

%.o: %.c
	@echo "CC $@"
	@${CC} -c ${CFLAGS} $< -o $@

%.lo: %.c
	@echo "CC $@"
	@${CC} -c -fPIC ${CFLAGS} $< -o $@ 
