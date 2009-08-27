
-- CREATE OR REPLACE FUNCTION vuln_sql_injection_direct( stmt text ) RETURNS VOID AS $$
--   BEGIN
--     EXECUTE stmt;
--     RETURN;
--   END;
-- $$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION test_simple( stmt text ) RETURNS VOID AS $$
  BEGIN
--    SELECT pg_catalog.version();
--    SELECT pg_catalog.version(stmt,found,3);
--
--    SELECT 'foo'::text;
--
--    SELECT usename,username FROM pg_user;
--    EXECUTE 'SELECT ' || stmt || ' FROM information_schema.tables';
--    INSERT INTO foo VALUES(stmt,2,3);
--    INSERT INTO foo VALUES(1,2),(3,4);
    UPDATE foo SET bar = true;

    RETURN;
  END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION minimal( arg1 text ) RETURNS VOID AS $$
  BEGIN

    RETURN;
  END;
$$ LANGUAGE plpgsql;


-- SELECT dump_sql_parse_tree($$SELECT foo.f1 AS f4,f2 FROM pg_user;$$);

SELECT dump_plpgsql_function('vuln_sql_injection_direct(text)'::regprocedure);
-- SELECT dump_plpgsql_function('test_simple(text)'::regprocedure);

