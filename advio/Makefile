include ../Make.defines

PROGS = oobselect 

all: ${PROGS}

oobselect: oobselect.cpp
	${CC} ${CFLAGS} ${LDDIR} $^ ${LDLIBS} -o $@
	
clean:
	rm -f ${PROGS} ${TEMPFILES} *.o