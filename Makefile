
PREFIX=/home/sven/diplom/local/

PGSERVERINCLUDE:= $(shell ${PREFIX}/bin/pg_config --includedir-server)
PGLIBDIR:= $(shell ${PREFIX}/bin/pg_config --pkglibdir)
PLPGSQLSRC:=/home/sven/diplom/postgresql/src/pl/plpgsql/src

CWD:= $(shell pwd)

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

test: install_functions
		${PREFIX}bin/psql < test.sql

check: install_functions
		${PREFIX}bin/psql -t -q < test.sql | xmllint --format -

install_functions: binary
		${PREFIX}bin/psql -q -c "CREATE OR REPLACE FUNCTION dump_plpgsql_function(oid) returns text AS '${CWD}/dump_module.so', 'dump_plpgsql_function' LANGUAGE C STRICT;"
		${PREFIX}bin/psql -q -c "CREATE OR REPLACE FUNCTION dump_sql_parse_tree(text) returns text AS '${CWD}/dump_module.so', 'dump_sql_parse_tree' LANGUAGE C STRICT;"

.PHONY: all clean binary test nice install_functions

