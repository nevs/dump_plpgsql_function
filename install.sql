
CREATE OR REPLACE FUNCTION dump_plpgsql_function(oid) returns text AS '/home/sven/diplom/dump_plpgsql_function/dump_module.so', 'dump_plpgsql_function' LANGUAGE C STRICT;
CREATE OR REPLACE FUNCTION dump_sql_parse_tree(text) returns text AS '/home/sven/diplom/dump_plpgsql_function/dump_module.so', 'dump_sql_parse_tree' LANGUAGE C STRICT;

