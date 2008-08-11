
CFLAGS= -I/usr/include/postgresql/8.3/server -I/home/sven/diplom/postgresql/src/pl/plpgsql/src -L/usr/lib/postgresql/8.3/lib

dump_plpgsql_function.so: dump_plpgsql_function.c
		gcc ${CFLAGS} -fpic -c dump_plpgsql_function.c
		gcc ${CFLAGS} -shared -o dump_plpgsql_function.so dump_plpgsql_function.o /usr/lib/postgresql/8.3/lib/plpgsql.so

clean:
		rm *.so *.o

test:
		psql < test.sql

