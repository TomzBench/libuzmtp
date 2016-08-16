# uzmtp makefile

PREFIX:=/usr/local
BINDIR:=${PREFIX}/bin
LIBDIR:=${PREFIX}/lib
INCDIR:=${PREFIX}/include
MANDIR:=${PREFIX}/share/man

IMGDIR:=build/img
SRCS:=$(wildcard ./src/os/linux/*.c)
SRCS+=$(wildcard ./src/os/wolfssl/*.c)
SRCS+=\
      ./src/uzmtp.c \
      ./src/uzmtp_msg.c \
      ./src/uzmtp_dealer.c \
      ./src/main.c

# SRCS+=$(wildcard ./src/*.c)

OBJS:=${SRCS:.c=.o}
RELOBJS:=${SRCS:.c=.lo}
HDRS:=${SRCS:.c=.h}
HDRS+= \
	./src/uzmtp_settings.h

all: $(IMGDIR)/libuzmtp.so $(IMGDIR)/libuzmtp

${IMGDIR}/libuzmtp.so: ${RELOBJS}
	@echo "LINK $@"
	@${CC} -shared ${RELOBJS} ${LDFLAGS} -o ${IMGDIR}/libuzmtp.so

${IMGDIR}/libuzmtp: ${OBJS} src/main.o
	@echo "LINK $@"
	@${CC} ${OBJS} ${LDFLAGS} -o ${IMGDIR}/libuzmtp

include libuzmtp.mk

.PHONY: cscope
cscope:
	find . \
		\( -name '*.c' -o -name '*.h' -o -name '*.hpp' -o -name '*.cpp' \) -print > cscope.files
		cscope -b 

.PHONY:
valgrind:
	valgrind \
		--tool=memcheck \
		--leak-check=full \
		--track-fds=yes \
		build/img/libuzmtp


.PHONY:
decruft:
	rm -f ${OBJS} ${RELOBJS}

.PHONY:
install:
	if [ -a $(IMGDIR)/libuzmtp ]; then cp $(IMGDIR)/libuzmtp $(BINDIR); fi;
	if [ -a $(IMGDIR)/libuzmtp.so ]; then cp $(IMGDIR)/libuzmtp.so $(LIBDIR); fi;
	mkdir /usr/local/include/uzmtp/ -p
	cp ${HDRS} /usr/local/include/uzmtp/

.PHONY: clean
clean:
	@echo "CLEAN"
	@rm -f ${OBJS} ${RELOBJS} ${IMGDIR}/*

