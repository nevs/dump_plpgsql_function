
PREFIX=/home/sven/diplom/local/

PGSERVERINCLUDE:= $(shell ${PREFIX}/bin/pg_config --includedir-server)
PGLIBDIR:= $(shell ${PREFIX}/bin/pg_config --pkglibdir)
PLPGSQLSRC:=/home/sven/diplom/postgresql/src/pl/plpgsql/src


INCLUDE= -I${PGSERVERINCLUDE} -I${PLPGSQLSRC}

CFLAGS= -fpic -Wall -g ${INCLUDE} -L${PREFIX}lib

HEADER_FILES= $(wildcard *.h)
SOURCE_FILES= $(wildcard *.c)
OBJECTS= $(patsubst %.c,%.o, ${SOURCE_FILES})

all: test

binary: dump_module.so

dump_module.so: Makefile ${OBJECTS}
		gcc ${CFLAGS} -shared -o dump_module.so ${OBJECTS} ${PGLIBDIR}/plpgsql.so

clean:
		rm *.so *.o

test: binary
		${PREFIX}bin/psql -e < test.sql

check: binary
		${PREFIX}bin/psql -t -q < test.sql | xmllint --format -

.PHONY: all clean binary test nice

