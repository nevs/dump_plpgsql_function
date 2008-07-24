
#include <postgres.h>
#include <utils/builtins.h>
#include <fmgr.h>
#include <plpgsql.h>

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

/*
 * CREATE OR REPLACE FUNCTION dump_plpgsql_function(text) returns text AS '/home/sven/diplom/dump_plpgsql_function/dump_plpgsql_function.so', 'dump_plpgsql_function' LANGUAGE C STRICT;
 *
 *
 */

extern text * cstring_to_text( char * );
extern PLpgSQL_function * plpgsql_compile( FunctionCallInfo fcinfo, bool forValidator );

PG_FUNCTION_INFO_V1(dump_plpgsql_function);

Datum dump_plpgsql_function( PG_FUNCTION_ARGS )
{
  text * t = PG_GETARG_TEXT_P( 0 );
  t = cstring_to_text( "<xml>" );
//  t = cstring_to_text( snprintf("1234567812345678",16,"%x",plpgsql_compile) );
  PG_RETURN_TEXT_P( t );
}


