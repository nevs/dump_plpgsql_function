
BEGIN;
  DROP FUNCTION simple_test();
COMMIT;

CREATE OR REPLACE FUNCTION simple_test() RETURNS TEXT AS $$
  DECLARE
  var_a TEXT;

  BEGIN

    var_a := 'version()';

    EXECUTE 'SELECT ' || var_a;

    RETURN 'version: ' || version();

  END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION dump_plpgsql_function(oid) returns text AS '/home/sven/diplom/dump_plpgsql_function/dump_plpgsql_function.so', 'dump_plpgsql_function' LANGUAGE C STRICT;
SELECT * from dump_plpgsql_function('simple_test()'::regprocedure);

