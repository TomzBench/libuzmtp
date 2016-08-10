CFLAGS?=-O0 -g -Wall -DNDEBUG -DUZMTP_USE_LINUX -I ../include 
CC:=cc -std=gnu11

all: uzmtp-client

uzmtp-client: uzmtp-client.o
	@echo "LINK $@"
	@${CC} uzmtp-client.o -o uzmtp-client -L../build/img -luzmtp

%.o: %.c
	@echo "CC $@"
	@${CC} -c ${CFLAGS} $< -o $@

.PHONY: clean
clean:
	@echo "CLEAN"
	@rm -f *.o uzmtp-client
