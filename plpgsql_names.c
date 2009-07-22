
#include "plpgsql.h"

const char * PLPGSQL_DTYPE_Names[] = {
  [PLPGSQL_DTYPE_VAR] = "VAR",
  [PLPGSQL_DTYPE_ROW] = "ROW",
  [PLPGSQL_DTYPE_REC] = "REC",
  [PLPGSQL_DTYPE_RECFIELD] = "RECFIELD",
  [PLPGSQL_DTYPE_ARRAYELEM] = "ARRAYELEM",
  [PLPGSQL_DTYPE_EXPR] = "EXPR",
  [PLPGSQL_DTYPE_TRIGARG] = "TRIGARG",
};

const char * PLPGSQL_STMT_Names[] = {
  [PLPGSQL_STMT_BLOCK] = "BLOCK",
  [PLPGSQL_STMT_ASSIGN] = "ASSIGN",
  [PLPGSQL_STMT_IF] = "IF",
  [PLPGSQL_STMT_CASE] = "CASE",
  [PLPGSQL_STMT_LOOP] = "LOOP",
  [PLPGSQL_STMT_WHILE] = "WHILE",
  [PLPGSQL_STMT_FORI] = "FORI",
  [PLPGSQL_STMT_FORS] = "FORS",
  [PLPGSQL_STMT_FORC] = "FORC",
  [PLPGSQL_STMT_EXIT] = "EXIT",
  [PLPGSQL_STMT_RETURN] = "RETURN",
  [PLPGSQL_STMT_RETURN_NEXT] = "RETURN_NEXT",
  [PLPGSQL_STMT_RETURN_QUERY] = "RETURN_QUERY",
  [PLPGSQL_STMT_RAISE] = "RAISE",
  [PLPGSQL_STMT_EXECSQL] = "EXECSQL",
  [PLPGSQL_STMT_DYNEXECUTE] = "DYNEXECUTE",
  [PLPGSQL_STMT_DYNFORS] = "DYNFORS",
  [PLPGSQL_STMT_GETDIAG] = "GETDIAG",
  [PLPGSQL_STMT_OPEN] = "OPEN",
  [PLPGSQL_STMT_FETCH] = "FETCH",
  [PLPGSQL_STMT_CLOSE] = "CLOSE",
  [PLPGSQL_STMT_PERFORM] = "PERFORM"
};

