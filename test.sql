
CREATE OR REPLACE FUNCTION vuln_sql_injection_direct( stmt text ) RETURNS VOID AS $$
  BEGIN
    EXECUTE stmt;
    RETURN;
  END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION test_simple( stmt text ) RETURNS VOID AS $$
  BEGIN
    SELECT version();
    SELECT 'foo'::text;
  
    SELECT usename,username FROM pg_user;
    INSERT INTO foo VALUES(1,2,3);

    RETURN;
  END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION test_debug( stmt text ) RETURNS VOID AS $$
  BEGIN
    INSERT INTO my_table VALUES(stmt);

    RETURN;
  END;
$$ LANGUAGE plpgsql;


-- SELECT dump_sql_parse_tree($$SELECT foo.f1 AS f4,f2 FROM pg_user;$$);

-- SELECT dump_plpgsql_function('vuln_sql_injection_direct(text)'::regprocedure);
-- SELECT dump_plpgsql_function('test_simple(text)'::regprocedure);
SELECT dump_plpgsql_function('test_debug(text)'::regprocedure);

