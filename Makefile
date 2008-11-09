
PREFIX=/home/sven/diplom/local/

PGSERVERINCLUDE=`${PREFIX}/bin/pg_config --includedir-server`
PGLIBDIR=`${PREFIX}/bin/pg_config --pkglibdir`
PLPGSQLSRC=/home/sven/diplom/postgresql/src/pl/plpgsql/src


INCLUDE= -I${PGSERVERINCLUDE} -I${PLPGSQLSRC}

CFLAGS= -fpic -Wall -g ${INCLUDE} -L${PREFIX}lib

HEADER_FILES= $(wildcard *.h)
SOURCE_FILES= $(wildcard *.c)
OBJECTS= $(patsubst %.c,%.o, ${SOURCE_FILES})

default: binary

binary: dump_module.so

dump_module.so: Makefile ${OBJECTS}
		gcc ${CFLAGS} -shared -o dump_module.so ${OBJECTS} ${PGLIBDIR}/plpgsql.so

clean:
		rm *.so *.o

test: binary
		${PREFIX}bin/psql < test.sql

nice: binary
		${PREFIX}bin/psql -t -q < test.sql | xmllint --format -

