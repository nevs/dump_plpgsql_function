
CFLAGS= -I/usr/include/postgresql/8.3/server

all:
		gcc ${CFLAGS} -fpic -c plpgsql_dumptree.c
		gcc ${CFLAGS} -shared -o plpgsql_dumptree.so plpgsql_dumptree.o

clean:
		rm plpgsql_dumptree.so plpgsql_dumptree.o

