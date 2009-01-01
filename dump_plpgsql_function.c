
#include <postgres.h>
#include <plpgsql.h>

#include "dump_plpgsql_function.h"
#include "dump_sql_parse_tree.h"
#include "string_helper.h"

#include "plpgsql_names.h"

#define CHILD_STMT( type, child ) child_statement( context, #child, (PLpgSQL_stmt *)((type *)node)->child )
#define CHILD_EXPR( type, child ) child_expression( context, #child, (PLpgSQL_expr *)((type *)node)->child )
#define TEXT_NODE( type, child ) if (((type *)node)->child) xml_textnode( context->dump, #child, "%s", (char *)((type *)node)->child )


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
static void dump_exception( FunctionDumpContext * context, PLpgSQL_exception * datum );

void child_statement( FunctionDumpContext * context, const char * tagname, PLpgSQL_stmt * statement )
{
  if ( statement ) {
    xml_tag_open( context->dump, tagname );
    dump_statement( context, statement );
    xml_tag_close( context->dump, tagname );
  }
}

void child_expression( FunctionDumpContext * context, const char * tagname, PLpgSQL_expr * expression )
{
  if ( expression ) {
    xml_tag_open( context->dump, tagname );
    dump_datum( context, (PLpgSQL_datum *) expression );
    xml_tag_close( context->dump, tagname );
  }
}

const char * dump_plpgsql_function_internal( DumpContext *dump, Oid func_oid )
{
  FunctionDumpContext * context = palloc0( sizeof( FunctionDumpContext ) );
  context->dump = dump;
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

  xml_tag_open( context->dump, "plpgsql_function_tree" );
  xml_textnode( context->dump, "name", "%s", func->fn_name );
  xml_textnode( context->dump, "oid", "%d", func->fn_oid );

  xml_tag_open( context->dump, "datums" );
  for (i = 0; i < func->ndatums; i++)
    dump_datum( context, func->datums[i] );
  xml_tag_close( context->dump, "datums" );

  child_statement( context, "action", (PLpgSQL_stmt *) func->action );
  xml_tag_close( context->dump, "plpgsql_function_tree" );
  return *context->dump->output;
}

static void dump_datum( FunctionDumpContext * context, PLpgSQL_datum * node )
{
  const char *tagname = PLPGSQL_DTYPE_Names[node->dtype] ? : "DATUM";

  xml_tag_open( context->dump, tagname );
  switch( node->dtype ) {
    case PLPGSQL_DTYPE_VAR:
      // FIXME add more fields
      TEXT_NODE( PLpgSQL_var, refname );
      CHILD_EXPR( PLpgSQL_var, default_val );
      break;
    case PLPGSQL_DTYPE_EXPR:
      if (((PLpgSQL_expr *)node)->query ) {
        xml_tag_open( context->dump, "params" );
        int i;
        for( i=0; i < ((PLpgSQL_expr *)node)->nparams; i++ ) {
          xml_tag_open( context->dump, "Param" );
          xml_textnode( context->dump, "index", "%d", ((PLpgSQL_expr *)node)->params[i] );
          xml_tag_close( context->dump, "Param" );
        }
        xml_tag_close( context->dump, "params" );
      }

      dump_sql_parse_tree_internal( context->dump, ((PLpgSQL_expr *)node)->query );
      break;
  }
  xml_tag_close( context->dump, tagname );
}

static void dump_statement( FunctionDumpContext * context, PLpgSQL_stmt *node )
{
  const char *tagname = PLPGSQL_STMT_Names[node->cmd_type] ? : "STATEMENT";
  ListCell *item;

  xml_tag_open( context->dump, tagname );
  switch( node->cmd_type ) {
    case PLPGSQL_STMT_BLOCK:           // 0
      TEXT_NODE( PLpgSQL_stmt_block, label );
      xml_tag_open( context->dump, "body" );
      foreach( item, ((PLpgSQL_stmt_block *)node)->body )
        dump_statement( context, lfirst( item ) );
      xml_tag_close( context->dump, "body" );
      if (((PLpgSQL_stmt_block *)node)->exceptions) {
        xml_tag_open( context->dump, "exceptions" );
        foreach( item, ((PLpgSQL_stmt_block *)node)->exceptions->exc_list )
          dump_exception( context, lfirst( item ) );
        xml_tag_close( context->dump, "exceptions" );
      }
      break;
    case PLPGSQL_STMT_RETURN:          // 10
      CHILD_EXPR( PLpgSQL_stmt_return, expr );
      break;
    case PLPGSQL_STMT_DYNEXECUTE:      // 15
      if (((PLpgSQL_stmt_dynexecute *)node)->params) {
        xml_tag_open( context->dump, "params" );
        foreach( item, ((PLpgSQL_stmt_dynexecute *)node)->params)
          dump_datum( context, lfirst( item ) );
        xml_tag_close( context->dump, "params" );
      }
      CHILD_EXPR( PLpgSQL_stmt_dynexecute, query );
      break;
    default:
      xml_tag( context->dump, "node", "cmd_type", "%d", node->cmd_type, NULL );
      break;

  };
  xml_tag_close( context->dump, tagname );
}

static void dump_exception( FunctionDumpContext * context, PLpgSQL_exception * datum )
{
}

