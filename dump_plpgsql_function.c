
#include <postgres.h>
#include <plpgsql.h>

#include "dump_plpgsql_function.h"
#include "dump_sql_parse_tree.h"
#include "string_helper.h"

#include "plpgsql_names.h"

#define ROOTNODENAME "function_tree"

#define CHILD_NODE( type, child ) child_statement( context, #child, (PLpgSQL_stmt *)((type *)node)->child )
#define TEXT_NODE( type, child ) xml_textnode( context->dump, #child, "%s", (char *)((type *)node)->child ? : "" )


/*
 * CREATE OR REPLACE FUNCTION dump_plpgsql_function(text) returns text AS '/home/sven/diplom/dump_plpgsql_function/dump_plpgsql_function.so', 'dump_plpgsql_function' LANGUAGE C STRICT;
 *
 */

typedef struct function_dump_context {
  PLpgSQL_function * func;
  DumpContext * dump;
} FunctionDumpContext;

static void dump_statement( FunctionDumpContext * context, PLpgSQL_stmt *stmt );
static void dump_datum( FunctionDumpContext * context, PLpgSQL_datum * datum );

void child_statement( FunctionDumpContext * context, const char * tagname, PLpgSQL_stmt * statement )
{
  if ( statement ) {
    xml_tag_open( context->dump, tagname, NULL );
    dump_statement( context, statement );
    xml_tag_close( context->dump, tagname );
  }
}

const char * dump_plpgsql_function_internal( Oid func_oid )
{
  FunctionDumpContext * context = palloc0( sizeof( FunctionDumpContext ) );
  context->dump = new_dump_context();
  int i;

  FunctionCallInfoData fake_fcinfo;
  FmgrInfo    flinfo;

  /*
   * Set up a fake fcinfo with just enough info to satisfy
   * plpgsql_compile().
   */
  MemSet(&fake_fcinfo, 0, sizeof(fake_fcinfo));
  MemSet(&flinfo, 0, sizeof(flinfo));
  fake_fcinfo.flinfo = &flinfo;
  flinfo.fn_oid = func_oid;
  flinfo.fn_mcxt = CurrentMemoryContext;
 
  PLpgSQL_function * func = plpgsql_compile(&fake_fcinfo, true );
  context->func = func;

  xml_tag_open( context->dump, ROOTNODENAME, NULL );
  xml_textnode( context->dump, "name", "%s", func->fn_name );
  xml_textnode( context->dump, "oid", "%d", func->fn_oid );

  xml_tag_open( context->dump, "datums", NULL );
  for (i = 0; i < func->ndatums; i++)
    dump_datum( context, func->datums[i] );
  xml_tag_close( context->dump, "datums" );

  child_statement( context, "action", (PLpgSQL_stmt *) func->action );
  xml_tag_close( context->dump, ROOTNODENAME );
  return *context->dump->output;
}

static void dump_datum( FunctionDumpContext * context, PLpgSQL_datum * node )
{
  const char *tagname = PLPGSQL_DTYPE_Names[node->dtype] ? : "DATUM";

  xml_tag_open( context->dump, tagname, NULL );
  switch( node->dtype ) {
    case PLPGSQL_DTYPE_VAR:
      xml_textnode( context->dump, "refname", "%s", ((PLpgSQL_var *)node)->refname );
      CHILD_NODE( PLpgSQL_var, default_val );
  }
  xml_tag_close( context->dump, tagname );
}

static void dump_statement( FunctionDumpContext * context, PLpgSQL_stmt *node )
{
  const char *tagname = PLPGSQL_STMT_Names[node->cmd_type] ? : "STATEMENT";
  ListCell *item;

  xml_tag_open( context->dump, tagname, NULL );
  switch( node->cmd_type ) {
    case PLPGSQL_STMT_BLOCK:
      TEXT_NODE( PLpgSQL_stmt_block, label );
      xml_tag_open( context->dump, "body", NULL );
      foreach( item, ((PLpgSQL_stmt_block *)node)->body ) {
        dump_statement( context, lfirst( item ) );
      }
      xml_tag_close( context->dump, "body" );
      break;

  };
  xml_tag_close( context->dump, tagname );
}

