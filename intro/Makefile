include ../Make.defines

PROGS = byteorder checkopts gethost

all: ${PROGS}

byteorder: byteorder.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@

checkopts: checkopts.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@

gethost: checkopts.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@
	
clean:
	rm -f ${PROGS} ${TEMPFILES} *.o