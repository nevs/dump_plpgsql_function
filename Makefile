
PLPGSQLSRC=/home/sven/diplom/postgresql/src/pl/plpgsql/src

PREFIX=/home/sven/diplom/local/

INCLUDE= -I${PREFIX}include/postgresql/server -I${PLPGSQLSRC}

CFLAGS= -fpic -Wall -g ${INCLUDE} -L${PREFIX}lib

default: nice

binary: dump_plpgsql_function.so

dump_plpgsql_function.so: Makefile dump_plpgsql_function.c
		gcc ${CFLAGS} -fpic -c string_helper.c
		gcc ${CFLAGS} -fpic -c dump_sql_parse_tree.c
		gcc ${CFLAGS} -fpic -c sql_parsetree_names.c
		gcc ${CFLAGS} -fpic -c dump_plpgsql_function.c
		gcc ${CFLAGS} -shared -o dump_plpgsql_function.so dump_plpgsql_function.o dump_sql_parse_tree.o sql_parsetree_names.o ${PREFIX}lib/postgresql/plpgsql.so

clean:
		rm *.so *.o

test: dump_plpgsql_function.so
		${PREFIX}bin/psql < test.sql

nice: dump_plpgsql_function.so
		${PREFIX}bin/psql -t -q < test.sql | xmllint --format -

