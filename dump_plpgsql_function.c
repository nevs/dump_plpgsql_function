
#include <postgres.h>
#include <plpgsql.h>
#include <utils/builtins.h>

#include "dump_plpgsql_function.h"
#include "dump_sql_parse_tree.h"
#include "string_helper.h"

#include "plpgsql_names.h"

#define CHILD_STMT( type, child ) child_statement( context, #child, (PLpgSQL_stmt *)((type *)node)->child )
#define CHILD_STMT_LIST( type, node, child ) \
  xml_tag_open( context->dump, #child ); \
  { \
  ListCell * item_macro; \
  foreach( item_macro, ((type *)(node))->child ) \
    dump_statement( context, lfirst( item_macro ) ); \
  } \
  xml_tag_close( context->dump, #child );

#define CHILD_EXPR( type, node, child ) child_expression( context, #child, (PLpgSQL_expr *)((type *)(node))->child )
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
static void dump_datatype( FunctionDumpContext * context, Oid typoid );

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
      xml_tag_open( context->dump, "argument" );

      xml_textnode( context->dump, "position", "%d", func->fn_argvarnos[i] );
      dump_datatype( context, func->fn_hashkey->argtypes[i] );
      xml_tag_close( context->dump, "argument" );
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

static void dump_datatype( FunctionDumpContext * context, Oid typoid )
{
  xml_tag_open( context->dump, "datatype" );
  xml_textnode( context->dump, "name", "%s", oid_datatype_name( typoid ));
  xml_textnode( context->dump, "oid", "%d", typoid );
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
      dump_datatype( context, ((PLpgSQL_var*)node)->datatype->typoid );
      BOOL_NODE( PLpgSQL_var, isconst );
      BOOL_NODE( PLpgSQL_var, notnull );
      CHILD_EXPR( PLpgSQL_var, node, default_val );
      if ( ((PLpgSQL_var*)node)->cursor_explicit_expr ) {
        CHILD_EXPR( PLpgSQL_var, node, cursor_explicit_expr );
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
      CHILD_EXPR( PLpgSQL_arrayelem, node, subscript );
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
      if ( ((PLpgSQL_expr *)node)->expr_simple_type ) {
        xml_tag_open( context->dump, "result_type" );
        dump_datatype( context, ((PLpgSQL_expr *)node)->expr_simple_type );
        xml_tag_close( context->dump, "result_type" );
      }
      

      dump_sql_parse_tree_internal( context->dump, ((PLpgSQL_expr *)node)->query );
      break;
    case PLPGSQL_DTYPE_TRIGARG:
      CHILD_EXPR( PLpgSQL_trigarg, node, argnum );
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
      CHILD_STMT_LIST( PLpgSQL_stmt_block, node, body );

      if (((PLpgSQL_stmt_block *)node)->exceptions) {
        xml_tag_open( context->dump, "exceptions" );
        foreach( item, ((PLpgSQL_stmt_block *)node)->exceptions->exc_list )
          dump_exception( context, lfirst( item ) );
        xml_tag_close( context->dump, "exceptions" );
      }
      break;
    case PLPGSQL_STMT_ASSIGN:          // 1
      CHILD_EXPR( PLpgSQL_stmt_assign, node, expr );
      NUMBER_NODE( PLpgSQL_stmt_assign, varno );
      break;
    case PLPGSQL_STMT_IF:              // 2
      CHILD_EXPR( PLpgSQL_stmt_if, node, cond );
      CHILD_STMT_LIST( PLpgSQL_stmt_if, node, true_body );
      CHILD_STMT_LIST( PLpgSQL_stmt_if, node, false_body );
      break;
    case PLPGSQL_STMT_CASE:            // 3
      CHILD_EXPR( PLpgSQL_stmt_case, node, t_expr );
      NUMBER_NODE( PLpgSQL_stmt_case, t_varno );

      xml_tag_open( context->dump, "case_when_list" );
      foreach( item, ((PLpgSQL_stmt_case *)node)->case_when_list ) {
        xml_tag_open( context->dump, "CASE_WHEN" );
        CHILD_EXPR( PLpgSQL_case_when, lfirst(item), expr );
        CHILD_STMT_LIST( PLpgSQL_case_when, lfirst(item), stmts );
        xml_tag_close( context->dump, "CASE_WHEN" );
      }
      xml_tag_close( context->dump, "case_when_list" );
      
      if (((PLpgSQL_stmt_case *)node)->have_else)
        CHILD_STMT_LIST( PLpgSQL_stmt_case, node, else_stmts )
      
      break;
    case PLPGSQL_STMT_LOOP:            // 4
      TEXT_NODE( PLpgSQL_stmt_loop, label );
      CHILD_STMT_LIST( PLpgSQL_stmt_loop, node, body );
      break;
    case PLPGSQL_STMT_WHILE:           // 5
      TEXT_NODE( PLpgSQL_stmt_while, label );
      CHILD_EXPR( PLpgSQL_stmt_while, node, cond );
      CHILD_STMT_LIST( PLpgSQL_stmt_while, node, body );
      break;
    case PLPGSQL_STMT_FORI:            // 6
      TEXT_NODE( PLpgSQL_stmt_fori, label );

      xml_tag_open( context->dump, "var" );
      dump_datum( context, ((PLpgSQL_stmt_fori *)node)->var );
      xml_tag_close( context->dump, "var" );

      CHILD_EXPR( PLpgSQL_stmt_fori, node, lower );
      CHILD_EXPR( PLpgSQL_stmt_fori, node, upper );
      CHILD_EXPR( PLpgSQL_stmt_fori, node, step );
      BOOL_NODE( PLpgSQL_stmt_fori, reverse );
      CHILD_STMT_LIST( PLpgSQL_stmt_fori, node, body );
      break;
    case PLPGSQL_STMT_FORS:            // 7
      TEXT_NODE( PLpgSQL_stmt_fors, label );

      xml_tag_open( context->dump, "rec" );
      dump_datum( context, ((PLpgSQL_stmt_fors *)node)->rec );
      xml_tag_close( context->dump, "rec" );

      xml_tag_open( context->dump, "row" );
      dump_datum( context, ((PLpgSQL_stmt_fors *)node)->row );
      xml_tag_close( context->dump, "row" );
      CHILD_STMT_LIST( PLpgSQL_stmt_fors, node, body );
      CHILD_EXPR( PLpgSQL_stmt_fors, node, query );
      break;
    case PLPGSQL_STMT_FORC:            // 8
      TEXT_NODE( PLpgSQL_stmt_forc, label );
      xml_tag_open( context->dump, "rec" );
      dump_datum( context, ((PLpgSQL_stmt_forc *)node)->rec );
      xml_tag_close( context->dump, "rec" );

      xml_tag_open( context->dump, "row" );
      dump_datum( context, ((PLpgSQL_stmt_forc *)node)->row );
      xml_tag_close( context->dump, "row" );

      CHILD_STMT_LIST( PLpgSQL_stmt_forc, node, body );
      NUMBER_NODE( PLpgSQL_stmt_forc, curvar );
      CHILD_EXPR( PLpgSQL_stmt_forc, node, argquery );
      break;
    case PLPGSQL_STMT_EXIT:            // 9
      BOOL_NODE( PLpgSQL_stmt_exit, is_exit );
      TEXT_NODE( PLpgSQL_stmt_exit, label );
      CHILD_EXPR( PLpgSQL_stmt_exit, node, cond );
      break;
    case PLPGSQL_STMT_RETURN:          // 10
      CHILD_EXPR( PLpgSQL_stmt_return, node, expr );
      NUMBER_NODE( PLpgSQL_stmt_return, retvarno );
      break;
    case PLPGSQL_STMT_RETURN_NEXT:     // 11
      CHILD_EXPR( PLpgSQL_stmt_return_next, node, expr );
      NUMBER_NODE( PLpgSQL_stmt_return_next, retvarno );
      break;
    case PLPGSQL_STMT_RETURN_QUERY:    // 12
      CHILD_EXPR( PLpgSQL_stmt_return_query, node, query );
      CHILD_EXPR( PLpgSQL_stmt_return_query, node, dynquery );

      if (((PLpgSQL_stmt_return_query *)node)->params) {
        xml_tag_open( context->dump, "params" );
        foreach( item, ((PLpgSQL_stmt_return_query *)node)->params)
          dump_datum( context, lfirst( item ) );
        xml_tag_close( context->dump, "params" );
      }
      break;
    case PLPGSQL_STMT_RAISE:           // 13
      NUMBER_NODE( PLpgSQL_stmt_raise, elog_level );
      TEXT_NODE( PLpgSQL_stmt_raise, condname );
      TEXT_NODE( PLpgSQL_stmt_raise, message );
      // FIXME params and options missing
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
      CHILD_EXPR( PLpgSQL_stmt_dynexecute, node, query );
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

