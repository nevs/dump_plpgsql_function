
CREATE OR REPLACE FUNCTION vuln_sql_injection_direct( stmt text ) RETURNS VOID AS $$
  BEGIN
    SELECT 'foo'::text;
    EXECUTE stmt;
    RETURN;
  END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION test_simple( stmt text ) RETURNS VOID AS $$
  BEGIN
    SELECT version();
  
    SELECT usename,username FROM pg_user;

    RETURN;
  END;
$$ LANGUAGE plpgsql;


-- SELECT dump_sql_parse_tree($$SELECT foo.f1 AS f4,f2 FROM pg_user;$$);

SELECT dump_plpgsql_function('vuln_sql_injection_direct(text)'::regprocedure);
SELECT dump_plpgsql_function('test_simple(text)'::regprocedure);

