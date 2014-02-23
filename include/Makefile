include ../Make.defines

AR	= ar
LIBMISC	= libunp.a
OBJS    = errwrap.o iowrap.o rw_fd.o sockwrap1.o sockwrap2.o sockwrap3.o

RANLIB     = ranlib

all:	${OBJS}
	${AR} rv ${LIBMISC} $?
	$(MAKE) clean
	${RANLIB} ${LIBMISC}

clean:
	rm -f *.o