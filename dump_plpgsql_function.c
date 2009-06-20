
#include <postgres.h>
#include <plpgsql.h>
#include <utils/builtins.h>

#include "dump_plpgsql_function.h"
#include "dump_sql_parse_tree.h"
#include "string_helper.h"

#include "plpgsql_names.h"

#define CHILD_STMT( type, child ) child_statement( context, #child, (PLpgSQL_stmt *)((type *)node)->child )
#define CHILD_EXPR( type, child ) child_expression( context, #child, (PLpgSQL_expr *)((type *)node)->child )
#define TEXT_NODE( type, child ) if (((type *)node)->child) xml_textnode( context->dump, #child, "%s", (char *)((type *)node)->child )
#define BOOL_NODE( type, child )  xml_textnode( context->dump, #child, "%d", ((type *)node)->child )
#define NUMBER_NODE( type, child )  xml_textnode( context->dump, #child, "%d", ((type *)node)->child )


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
static void dump_type( FunctionDumpContext * context, PLpgSQL_type * type );

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

static const char * oid_datatype_name( Oid oid )
{
  return DatumGetCString( DirectFunctionCall1( regtypeout, oid ) );
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

  xml_tag_open( context->dump, "plpgsql_function" );
  xml_textnode( context->dump, "name", "%s", func->fn_name );
  xml_textnode( context->dump, "oid", "%d", func->fn_oid );

  xml_tag_open( context->dump, "arguments" );
    for (i = 0; i < func->fn_nargs; i++) {
      xml_tag( context->dump, "argument",
               "position", "%d", i,
               "datum", "%d", func->fn_argvarnos[i],
               "oid", "%d", func->fn_hashkey->argtypes[i],
               "datatype", "%s", oid_datatype_name( func->fn_hashkey->argtypes[i] ),
               NULL );
    }
  xml_tag_close( context->dump, "arguments" );

  xml_tag_open( context->dump, "datums" );
  for (i = 0; i < func->ndatums; i++)
    dump_datum( context, func->datums[i] );
  xml_tag_close( context->dump, "datums" );

  child_statement( context, "action", (PLpgSQL_stmt *) func->action );
  xml_tag_close( context->dump, "plpgsql_function" );
  return *context->dump->output;
}

static void dump_type( FunctionDumpContext * context, PLpgSQL_type * type )
{
  xml_tag_open( context->dump, "datatype" );
  xml_textnode( context->dump, "name", "%s", type->typname );
  xml_textnode( context->dump, "oid", "%d", type->typoid );
  xml_tag_close( context->dump, "datatype" );
}

static void dump_datum( FunctionDumpContext * context, PLpgSQL_datum * node )
{
  const char *tagname = PLPGSQL_DTYPE_Names[node->dtype] ? : "DATUM";
  int i;

  xml_tag_open( context->dump, tagname );
  switch( node->dtype ) {
    case PLPGSQL_DTYPE_VAR:
      TEXT_NODE( PLpgSQL_var, refname );
      dump_type( context, ((PLpgSQL_var*)node)->datatype );
      BOOL_NODE( PLpgSQL_var, isconst );
      BOOL_NODE( PLpgSQL_var, notnull );
      CHILD_EXPR( PLpgSQL_var, default_val );
      if ( ((PLpgSQL_var*)node)->cursor_explicit_expr ) {
        CHILD_EXPR( PLpgSQL_var, cursor_explicit_expr );
        BOOL_NODE( PLpgSQL_var, cursor_explicit_argrow );
        BOOL_NODE( PLpgSQL_var, cursor_options );
      }
      NUMBER_NODE( PLpgSQL_var, value );
      BOOL_NODE( PLpgSQL_var, isnull );
      BOOL_NODE( PLpgSQL_var, freeval );
      break;
    case PLPGSQL_DTYPE_ROW:
      TEXT_NODE( PLpgSQL_row, refname );
      xml_textnode( context->dump, "oid", "%d", ((PLpgSQL_row *)node)->rowtupdesc->tdtypeid );
      xml_tag_open( context->dump, "fields" );
      for( i=0; i < ((PLpgSQL_row *)node)->nfields; i++ ) {
        xml_tag_open( context->dump, "field" );
        xml_textnode( context->dump, "fieldname", "%s", ((PLpgSQL_row *)node)->fieldnames[i] );
        xml_textnode( context->dump, "oid", "%d", ((PLpgSQL_row *)node)->rowtupdesc->attrs[i]->atttypid );
        xml_textnode( context->dump, "datatype", "%s", oid_datatype_name(((PLpgSQL_row *)node)->rowtupdesc->attrs[i]->atttypid) );
        xml_tag_close( context->dump, "field" );
      }
      xml_tag_close( context->dump, "fields" );
      break;
    case PLPGSQL_DTYPE_REC:
      TEXT_NODE( PLpgSQL_rec, refname );
      xml_textnode( context->dump, "oid", "%d", ((PLpgSQL_rec *)node)->tupdesc->tdtypeid );
      xml_tag_open( context->dump, "fields" );
      for( i=0; i < ((PLpgSQL_rec *)node)->tupdesc->natts; i++ ) {
        xml_tag_open( context->dump, "field" );
        xml_textnode( context->dump, "fieldname", "%s", ((PLpgSQL_rec *)node)->tupdesc->attrs[i]->attname );
        xml_textnode( context->dump, "oid", "%d", ((PLpgSQL_rec *)node)->tupdesc->attrs[i]->atttypid );
        xml_textnode( context->dump, "datatype", "%s", oid_datatype_name(((PLpgSQL_rec *)node)->tupdesc->attrs[i]->atttypid) );
        xml_tag_close( context->dump, "field" );
      }
      xml_tag_close( context->dump, "fields" );
      break;
    case PLPGSQL_DTYPE_RECFIELD:
      // FIXME incomplete
      TEXT_NODE( PLpgSQL_recfield, fieldname );
      break;
    case PLPGSQL_DTYPE_ARRAYELEM:
      CHILD_EXPR( PLpgSQL_arrayelem, subscript );
      NUMBER_NODE( PLpgSQL_arrayelem, arrayparentno );
      break;
    case PLPGSQL_DTYPE_EXPR:
      // FIXME incomplete
      if (((PLpgSQL_expr *)node)->query ) {
        TEXT_NODE( PLpgSQL_expr, query );
        xml_tag_open( context->dump, "params" );
        for( i=0; i < ((PLpgSQL_expr *)node)->nparams; i++ ) {
          xml_tag_open( context->dump, "Param" );
          xml_textnode( context->dump, "index", "%d", ((PLpgSQL_expr *)node)->params[i] );
          if (((PLpgSQL_expr *)node)->plan) {
            
          }
//          xml_textnode( context->dump, "datatype_oid", "%d", ((PLpgSQL_expr *)node)->plan_argtypes[i] );
//          xml_textnode( context->dump, "datatype", "%s", oid_datatype_name(((PLpgSQL_expr *)node)->plan_argtypes[i]) );
          xml_tag_close( context->dump, "Param" );
        }
        xml_tag_close( context->dump, "params" );
      }
      if ( ((PLpgSQL_expr *)node)->expr_simple_expr ) {
        xml_textnode( context->dump, "result_type_oid", "%d", ((PLpgSQL_expr *)node)->expr_simple_type );
        xml_textnode( context->dump, "result_type", "%d", oid_datatype_name(((PLpgSQL_expr *)node)->expr_simple_type) );
      }
      

      dump_sql_parse_tree_internal( context->dump, ((PLpgSQL_expr *)node)->query );
      break;
    case PLPGSQL_DTYPE_TRIGARG:
      CHILD_EXPR( PLpgSQL_trigarg, argnum );
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
    case PLPGSQL_STMT_ASSIGN:          // 1
      CHILD_EXPR( PLpgSQL_stmt_assign, expr );
      break;
    case PLPGSQL_STMT_RETURN:          // 10
      CHILD_EXPR( PLpgSQL_stmt_return, expr );
      break;
    case PLPGSQL_STMT_EXECSQL:         // 14
      child_expression( context, "sqlstmt", ((PLpgSQL_stmt_execsql *)node)->sqlstmt);
      
      break;
    case PLPGSQL_STMT_DYNEXECUTE:      // 15
      // FIXME incomplete 
      if (((PLpgSQL_stmt_dynexecute *)node)->params) {
        xml_tag_open( context->dump, "params" );
        foreach( item, ((PLpgSQL_stmt_dynexecute *)node)->params)
          dump_datum( context, lfirst( item ) );
        xml_tag_close( context->dump, "params" );
      }
      CHILD_EXPR( PLpgSQL_stmt_dynexecute, query );
      break;
    default:
      // FIXME
      xml_tag( context->dump, "node", "cmd_type", "%d", node->cmd_type, NULL );
      break;

  };
  xml_tag_close( context->dump, tagname );
}

static void dump_exception( FunctionDumpContext * context, PLpgSQL_exception * datum )
{
  xml_tag_open( context->dump, "Exception" );
  // FIXME incomplete 
  xml_tag_close( context->dump, "Exception" );
}

