
CREATE OR REPLACE FUNCTION vuln_sql_injection_direct( stmt text ) RETURNS VOID AS $$
  BEGIN
    EXECUTE stmt;
    RETURN;
  END;
$$ LANGUAGE plpgsql;


-- SELECT dump_sql_parse_tree($$SELECT foo.f1 AS f4,f2 FROM pg_user;$$);

SELECT dump_plpgsql_function('vuln_sql_injection_direct(text)'::regprocedure);

