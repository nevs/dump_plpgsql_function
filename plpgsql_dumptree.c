
#include "postgres.h"
#include "fmgr.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(plpgsql_dumptree);

Datum plpgsql_dumptree( PG_FUNCTION_ARGS )
{
  text *t = PG_GETARG_TEXT_P( 0 );
  PG_RETURN_TEXT_P( t );
}


