include ../Make.defines

PROGS = daytimecli daytimesrv oobcli oobserv tcpicli tcpiserv tcppollserv tcpselectserv udpcli udpserv 

all: ${PROGS}

daytimecli: daytimecli.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@

daytimesrv: daytimesrv.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@

oobcli: oobcli.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@

oobserv: oobserv.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@

tcpicli: tcpicli.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@

tcpiserv: tcpiserv.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@

tcppollserv: tcppollserv.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@


tcpselectserv: tcpselectserv.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@

udpcli: udpcli.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@

udpserv: udpserv.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@

clean:
	rm -f ${PROGS} ${TEMPFILES} *.o