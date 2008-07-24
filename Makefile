
CFLAGS= -I/usr/include/postgresql/8.3/server -I/home/sven/diplom/postgresql/src/pl/plpgsql/src

plpgsql_dumptree.so: plpgsql_dumptree.c
		gcc ${CFLAGS} -fpic -c plpgsql_dumptree.c
		gcc ${CFLAGS} -shared -o plpgsql_dumptree.so plpgsql_dumptree.o

clean:
		rm plpgsql_dumptree.so plpgsql_dumptree.o

