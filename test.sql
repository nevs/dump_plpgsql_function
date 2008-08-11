
CREATE OR REPLACE FUNCTION dump_plpgsql_function(text) returns text AS '/home/sven/diplom/dump_plpgsql_function/dump_plpgsql_function.so', 'dump_plpgsql_function' LANGUAGE C STRICT;
SELECT * from dump_plpgsql_function('<xml2>');

