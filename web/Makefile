include ../Make.defines

PROGS = clienttest prefork prethread web 

all: ${PROGS}

clienttest: clienttest.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@

prefork: prefork.cpp server.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@

prethread: prethread.cpp server.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@

web: web.cpp server.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@
	
clean:
	rm -f ${PROGS} ${TEMPFILES} *.o