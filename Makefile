
PLPGSQLSRC=/home/sven/diplom/postgresql/src/pl/plpgsql/src

PREFIX=/home/sven/diplom/local/

INCLUDE= -I${PREFIX}include/postgresql/server -I${PLPGSQLSRC}

CFLAGS= -Wall -g ${INCLUDE} -L${PREFIX}lib

default: nice

binary: dump_plpgsql_function.so

dump_plpgsql_function.so: Makefile dump_plpgsql_function.c
		gcc ${CFLAGS} -fpic -c dump_plpgsql_function.c
		gcc ${CFLAGS} -shared -o dump_plpgsql_function.so dump_plpgsql_function.o ${PREFIX}lib/postgresql/plpgsql.so

clean:
		rm *.so *.o

test: dump_plpgsql_function.so
		${PREFIX}bin/psql < test.sql

nice: dump_plpgsql_function.so
		${PREFIX}bin/psql -t -q < test.sql | xmllint --format -

