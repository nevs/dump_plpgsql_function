
#include <postgres.h>
#include <fmgr.h>
#include <utils/builtins.h>

#include "string_helper.h"
#include "dump_plpgsql_function.h"
#include "dump_sql_parse_tree.h"


PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(dump_sql_parse_tree);

Datum dump_sql_parse_tree( PG_FUNCTION_ARGS )
{
  char * query = text_to_cstring( PG_GETARG_TEXT_P( 0 ) );
  const char * output = dump_sql_parse_tree_internal( query );

  if ( output )
    PG_RETURN_TEXT_P( cstring_to_text( output ) );
  else 
    PG_RETURN_NULL();
}


PG_FUNCTION_INFO_V1(dump_plpgsql_function);

Datum dump_plpgsql_function( PG_FUNCTION_ARGS )
{
  const char * result = dump_plpgsql_function_internal( PG_GETARG_OID( 0 ) );

  PG_RETURN_TEXT_P( cstring_to_text( result ) );
}

void _PG_init()
{
  string_helper_init();
}

