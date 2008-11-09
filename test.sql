
BEGIN;
  DROP FUNCTION simple_test();
COMMIT;

CREATE OR REPLACE FUNCTION simple_test() RETURNS TEXT AS $$
  DECLARE
  var_a TEXT;

  BEGIN

    var_a := 'version()';

    EXECUTE 'SELECT ' OPERATOR(pg_catalog.||) var_a;

    RETURN 'version: ' || version();

  END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION dump_plpgsql_function(oid) returns text AS '/home/sven/diplom/dump_plpgsql_function/dump_module.so', 'dump_plpgsql_function' LANGUAGE C STRICT;
CREATE OR REPLACE FUNCTION dump_sql_parse_tree(text) returns text AS '/home/sven/diplom/dump_plpgsql_function/dump_module.so', 'dump_sql_parse_tree' LANGUAGE C STRICT;

--SELECT * from dump_plpgsql_function('simple_test()'::regprocedure);

SELECT * from dump_sql_parse_tree($$SELECT 'chunky' || 'bacon'$$);


