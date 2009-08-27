
#include <postgres.h>
#include <plpgsql.h>
#include <utils/builtins.h>

#include "dump_plpgsql_function.h"
#include "dump_sql_parse_tree.h"
#include "string_helper.h"

#include "plpgsql_names.h"
#include "sql_parsetree_names.h"

#define CHILD_DATUM( type, node, child ) \
  if (((type *)node)->child) { \
    xml_tag_open( context->dump, #child ); \
    dump_datum( context, (PLpgSQL_datum *)((type *)node)->child ); \
    xml_tag_close( context->dump, #child ); \
  }

#define CHILD_DATUM_LIST( type, node, child ) \
  if (((type *)node)->child) { \
    ListCell * item_macro; \
    xml_tag_open( context->dump, #child ); \
    foreach( item_macro, ((type *)(node))->child ) \
      dump_datum( context, (PLpgSQL_datum *) lfirst(item_macro)); \
    xml_tag_close( context->dump, #child ); \
  }

#define CHILD_STMT( type, node, child ) \
  if (((type *)node)->child) { \
    xml_tag_open( context->dump, #child ); \
    dump_statement( context, (PLpgSQL_stmt *)((type *)node)->child ); \
    xml_tag_close( context->dump, #child ); \
  }

#define CHILD_STMT_LIST( type, node, child ) \
  xml_tag_open( context->dump, #child ); \
  { \
  ListCell * item_macro; \
  foreach( item_macro, ((type *)(node))->child ) \
    dump_statement( context, lfirst( item_macro ) ); \
  } \
  xml_tag_close( context->dump, #child );

#define TEXT_NODE( type, node, child ) if (((type *)node)->child) xml_textnode( context->dump, #child, "%s", (char *)((type *)node)->child )
#define BOOL_NODE( type, node, child )  xml_textnode( context->dump, #child, "%d", ((type *)node)->child )
#define NUMBER_NODE( type, node, child )  xml_textnode( context->dump, #child, "%d", ((type *)node)->child )


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
static void dump_tupledesc( FunctionDumpContext * context, TupleDesc tupdesc );

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
      xml_textnode( context->dump, "name", "$%d", i + 1 );
      dump_datatype( context, func->fn_hashkey->argtypes[i] );
      xml_tag_close( context->dump, "argument" );
    }
  xml_tag_close( context->dump, "arguments" );

  xml_tag_open( context->dump, "datums" );
  for (i = 0; i < func->ndatums; i++)
    dump_datum( context, func->datums[i] );
  xml_tag_close( context->dump, "datums" );

  CHILD_STMT( PLpgSQL_function, func, action );
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
  NUMBER_NODE( PLpgSQL_datum, node, dno );
  switch( node->dtype ) {
    case PLPGSQL_DTYPE_VAR:
      TEXT_NODE( PLpgSQL_var, node, refname );
      NUMBER_NODE( PLpgSQL_var, node, lineno );
      dump_datatype( context, ((PLpgSQL_var*)node)->datatype->typoid );
      BOOL_NODE( PLpgSQL_var, node, isconst );
      BOOL_NODE( PLpgSQL_var, node, notnull );
      CHILD_DATUM( PLpgSQL_var, node, default_val );
      if ( ((PLpgSQL_var*)node)->cursor_explicit_expr ) {
        CHILD_DATUM( PLpgSQL_var, node, cursor_explicit_expr );
        BOOL_NODE( PLpgSQL_var, node, cursor_explicit_argrow );
        BOOL_NODE( PLpgSQL_var, node, cursor_options );
      }
      NUMBER_NODE( PLpgSQL_var, node, value );
      BOOL_NODE( PLpgSQL_var, node, isnull );
      BOOL_NODE( PLpgSQL_var, node, freeval );
      break;
    case PLPGSQL_DTYPE_ROW:
      TEXT_NODE( PLpgSQL_row, node, refname );
      NUMBER_NODE( PLpgSQL_row, node, lineno );

      xml_tag_open( context->dump, "rowtupdesc" );
      dump_tupledesc( context, ((PLpgSQL_row *)node)->rowtupdesc);
      xml_tag_close( context->dump, "rowtupdesc" );

      xml_tag_open( context->dump, "fields" );
      for( i=0; i < ((PLpgSQL_row *)node)->nfields; i++ ) {
        xml_tag_open( context->dump, "field" );
        xml_textnode( context->dump, "fieldname", "%s", ((PLpgSQL_row *)node)->fieldnames[i] );
        xml_textnode( context->dump, "varno", "%d", ((PLpgSQL_row *)node)->varnos[i] );
        xml_tag_close( context->dump, "field" );
      }
      xml_tag_close( context->dump, "fields" );
      break;
    case PLPGSQL_DTYPE_REC:
      TEXT_NODE( PLpgSQL_rec, node, refname );
      NUMBER_NODE( PLpgSQL_rec, node, lineno );

      xml_tag_open( context->dump, "tupdesc" );
      dump_tupledesc( context, ((PLpgSQL_rec *)node)->tupdesc);
      xml_tag_close( context->dump, "tupdesc" );
      BOOL_NODE( PLpgSQL_rec, node, freetup );
      BOOL_NODE( PLpgSQL_rec, node, freetupdesc );
      break;
    case PLPGSQL_DTYPE_RECFIELD:
      TEXT_NODE( PLpgSQL_recfield, node, fieldname );
      NUMBER_NODE( PLpgSQL_recfield, node, recparentno );
      break;
    case PLPGSQL_DTYPE_ARRAYELEM:
      CHILD_DATUM( PLpgSQL_arrayelem, node, subscript );
      NUMBER_NODE( PLpgSQL_arrayelem, node, arrayparentno );
      break;
    case PLPGSQL_DTYPE_EXPR:
      TEXT_NODE( PLpgSQL_expr, node, query );
      dump_sql_parse_tree_internal( context->dump, ((PLpgSQL_expr *)node)->query );

      if (((PLpgSQL_expr *)node)->expr_simple_expr) {
        xml_textnode( context->dump, "expr_simple_type", "%d", ((PLpgSQL_expr *)node)->expr_simple_type );
        xml_textnode( context->dump, "expr_simple_type_name", "%d", oid_datatype_name(((PLpgSQL_expr *)node)->expr_simple_type) );
      }

      xml_tag_open( context->dump, "params" );
      for( i=0; i < ((PLpgSQL_expr *)node)->nparams; i++ ) {
        xml_tag_open( context->dump, "Param" );
        xml_textnode( context->dump, "index", "%d", ((PLpgSQL_expr *)node)->params[i] );
        xml_tag_close( context->dump, "Param" );
      }
      xml_tag_close( context->dump, "params" );
      break;
    case PLPGSQL_DTYPE_TRIGARG:
      CHILD_DATUM( PLpgSQL_trigarg, node, argnum );
      break;
  }
  xml_tag_close( context->dump, tagname );
}

static void dump_statement( FunctionDumpContext * context, PLpgSQL_stmt *node )
{
  const char *tagname = PLPGSQL_STMT_Names[node->cmd_type] ? : "STATEMENT";
  ListCell *item;

  xml_tag_open( context->dump, tagname );
  NUMBER_NODE( PLpgSQL_stmt, node, lineno );
  switch( node->cmd_type ) {
    case PLPGSQL_STMT_BLOCK:           // 0
      TEXT_NODE( PLpgSQL_stmt_block, node, label );
      CHILD_STMT_LIST( PLpgSQL_stmt_block, node, body );
      NUMBER_NODE( PLpgSQL_stmt_block, node, n_initvars );

      // FIXME exception block
      if (((PLpgSQL_stmt_block *)node)->exceptions) {
        xml_tag_open( context->dump, "exceptions" );
        foreach( item, ((PLpgSQL_stmt_block *)node)->exceptions->exc_list )
          dump_exception( context, lfirst( item ) );
        xml_tag_close( context->dump, "exceptions" );
      }
      break;
    case PLPGSQL_STMT_ASSIGN:          // 1
      CHILD_DATUM( PLpgSQL_stmt_assign, node, expr );
      NUMBER_NODE( PLpgSQL_stmt_assign, node, varno );
      break;
    case PLPGSQL_STMT_IF:              // 2
      CHILD_DATUM( PLpgSQL_stmt_if, node, cond );
      CHILD_STMT_LIST( PLpgSQL_stmt_if, node, true_body );
      CHILD_STMT_LIST( PLpgSQL_stmt_if, node, false_body );
      break;
    case PLPGSQL_STMT_CASE:            // 3
      CHILD_DATUM( PLpgSQL_stmt_case, node, t_expr );
      NUMBER_NODE( PLpgSQL_stmt_case, node, t_varno );

      xml_tag_open( context->dump, "case_when_list" );
      foreach( item, ((PLpgSQL_stmt_case *)node)->case_when_list ) {
        xml_tag_open( context->dump, "CASE_WHEN" );
        CHILD_DATUM( PLpgSQL_case_when, lfirst(item), expr );
        CHILD_STMT_LIST( PLpgSQL_case_when, lfirst(item), stmts );
        xml_tag_close( context->dump, "CASE_WHEN" );
      }
      xml_tag_close( context->dump, "case_when_list" );
      
      if (((PLpgSQL_stmt_case *)node)->have_else)
        CHILD_STMT_LIST( PLpgSQL_stmt_case, node, else_stmts )
      
      break;
    case PLPGSQL_STMT_LOOP:            // 4
      TEXT_NODE( PLpgSQL_stmt_loop, node, label );
      CHILD_STMT_LIST( PLpgSQL_stmt_loop, node, body );
      break;
    case PLPGSQL_STMT_WHILE:           // 5
      TEXT_NODE( PLpgSQL_stmt_while, node, label );
      CHILD_DATUM( PLpgSQL_stmt_while, node, cond );
      CHILD_STMT_LIST( PLpgSQL_stmt_while, node, body );
      break;
    case PLPGSQL_STMT_FORI:            // 6 for statement with integer loop var
      TEXT_NODE( PLpgSQL_stmt_fori, node, label );
      CHILD_DATUM( PLpgSQL_stmt_fori, node, var );
      CHILD_DATUM( PLpgSQL_stmt_fori, node, lower );
      CHILD_DATUM( PLpgSQL_stmt_fori, node, upper );
      CHILD_DATUM( PLpgSQL_stmt_fori, node, step );
      BOOL_NODE( PLpgSQL_stmt_fori, node, reverse );
      CHILD_STMT_LIST( PLpgSQL_stmt_fori, node, body );
      break;
    case PLPGSQL_STMT_FORS:            // 7 for statement over select
      TEXT_NODE( PLpgSQL_stmt_fors, node, label );
      CHILD_DATUM( PLpgSQL_stmt_fors, node, rec );
      CHILD_DATUM( PLpgSQL_stmt_fors, node, row );
      CHILD_STMT_LIST( PLpgSQL_stmt_fors, node, body );
      CHILD_DATUM( PLpgSQL_stmt_fors, node, query );
      break;
    case PLPGSQL_STMT_FORC:            // 8 for statement over cursor
      TEXT_NODE( PLpgSQL_stmt_forc, node, label );
      CHILD_DATUM( PLpgSQL_stmt_forc, node, rec );
      CHILD_DATUM( PLpgSQL_stmt_forc, node, row );
      CHILD_STMT_LIST( PLpgSQL_stmt_forc, node, body );
      NUMBER_NODE( PLpgSQL_stmt_forc, node, curvar );
      CHILD_DATUM( PLpgSQL_stmt_forc, node, argquery );
      break;
    case PLPGSQL_STMT_EXIT:            // 9
      BOOL_NODE( PLpgSQL_stmt_exit, node, is_exit );
      TEXT_NODE( PLpgSQL_stmt_exit, node, label );
      CHILD_DATUM( PLpgSQL_stmt_exit, node, cond );
      break;
    case PLPGSQL_STMT_RETURN:          // 10
      CHILD_DATUM( PLpgSQL_stmt_return, node, expr );
      NUMBER_NODE( PLpgSQL_stmt_return, node, retvarno );
      break;
    case PLPGSQL_STMT_RETURN_NEXT:     // 11
      CHILD_DATUM( PLpgSQL_stmt_return_next, node, expr );
      NUMBER_NODE( PLpgSQL_stmt_return_next, node, retvarno );
      break;
    case PLPGSQL_STMT_RETURN_QUERY:    // 12
      CHILD_DATUM( PLpgSQL_stmt_return_query, node, query );
      CHILD_DATUM( PLpgSQL_stmt_return_query, node, dynquery );
      CHILD_DATUM_LIST( PLpgSQL_stmt_return_query, node, params );
      break;
    case PLPGSQL_STMT_RAISE:           // 13
      NUMBER_NODE( PLpgSQL_stmt_raise, node, elog_level );
      TEXT_NODE( PLpgSQL_stmt_raise, node, condname );
      TEXT_NODE( PLpgSQL_stmt_raise, node, message );
      CHILD_DATUM_LIST( PLpgSQL_stmt_raise, node, params );
      xml_tag_open( context->dump, "options" );
      foreach( item, ((PLpgSQL_stmt_raise *)(node))->options ) {
        xml_tag_open( context->dump, "RAISE_OPTION" );
        NUMBER_NODE( PLpgSQL_raise_option, lfirst(item), opt_type ); 
        CHILD_DATUM( PLpgSQL_raise_option, lfirst(item), expr)
        xml_tag_close( context->dump, "RAISE_OPTION" );
      }
      xml_tag_close( context->dump, "options" );
      break;
    case PLPGSQL_STMT_EXECSQL:         // 14
      CHILD_DATUM( PLpgSQL_stmt_execsql, node, sqlstmt );
      BOOL_NODE( PLpgSQL_stmt_execsql, node, mod_stmt );
      BOOL_NODE( PLpgSQL_stmt_execsql, node, into );
      BOOL_NODE( PLpgSQL_stmt_execsql, node, strict );
      CHILD_DATUM( PLpgSQL_stmt_execsql, node, rec );
      CHILD_DATUM( PLpgSQL_stmt_execsql, node, row );
      break;
    case PLPGSQL_STMT_DYNEXECUTE:      // 15
      CHILD_DATUM( PLpgSQL_stmt_dynexecute, node, query );
      BOOL_NODE( PLpgSQL_stmt_dynexecute, node, into );
      BOOL_NODE( PLpgSQL_stmt_dynexecute, node, strict );
      CHILD_DATUM( PLpgSQL_stmt_dynexecute, node, rec );
      CHILD_DATUM( PLpgSQL_stmt_dynexecute, node, row );
      CHILD_DATUM_LIST( PLpgSQL_stmt_dynexecute, node, params );
      break;
    case PLPGSQL_STMT_DYNFORS:         // 16
      TEXT_NODE( PLpgSQL_stmt_dynfors, node, label );
      CHILD_DATUM( PLpgSQL_stmt_dynfors, node, rec );
      CHILD_DATUM( PLpgSQL_stmt_dynfors, node, row );
      CHILD_STMT_LIST( PLpgSQL_stmt_dynfors, node, body );
      CHILD_DATUM( PLpgSQL_stmt_dynfors, node, query );
      CHILD_DATUM_LIST( PLpgSQL_stmt_dynfors, node, params );
      break;
    case PLPGSQL_STMT_GETDIAG:
      xml_tag_open( context->dump, "diag_items" );
      foreach( item, ((PLpgSQL_stmt_getdiag *)(node))->diag_items ) {
        xml_tag_open( context->dump, "DIAG_ITEM" );
        NUMBER_NODE( PLpgSQL_diag_item, lfirst(item), kind ); 
        NUMBER_NODE( PLpgSQL_diag_item, lfirst(item), target ); 
        xml_tag_close( context->dump, "DIAG_ITEM" );
      }
      xml_tag_close( context->dump, "diag_items" );
      break;
    case PLPGSQL_STMT_OPEN:
      NUMBER_NODE( PLpgSQL_stmt_open, node, curvar );
      NUMBER_NODE( PLpgSQL_stmt_open, node, cursor_options );
      CHILD_DATUM( PLpgSQL_stmt_open, node, returntype );
      CHILD_DATUM( PLpgSQL_stmt_open, node, argquery );
      CHILD_DATUM( PLpgSQL_stmt_open, node, query );
      CHILD_DATUM( PLpgSQL_stmt_open, node, dynquery );
      break;
    case PLPGSQL_STMT_FETCH:
      CHILD_DATUM( PLpgSQL_stmt_fetch, node, rec );
      CHILD_DATUM( PLpgSQL_stmt_fetch, node, row );
      NUMBER_NODE( PLpgSQL_stmt_fetch, node, curvar );
      xml_textnode( context->dump, "direction", "%s", FetchDirection_Names[((PLpgSQL_stmt_fetch*)node)->direction] );
      NUMBER_NODE( PLpgSQL_stmt_fetch, node, how_many );
      CHILD_DATUM( PLpgSQL_stmt_fetch, node, expr );
      BOOL_NODE( PLpgSQL_stmt_fetch, node, is_move );
    case PLPGSQL_STMT_CLOSE:
      NUMBER_NODE( PLpgSQL_stmt_close, node, curvar );
      break;
    case PLPGSQL_STMT_PERFORM:
      CHILD_DATUM( PLpgSQL_stmt_perform, node, expr );
      break;
//    default:
//      xml_tag( context->dump, "node", "cmd_type", "%d", node->cmd_type, NULL );
//      break;

  };
  xml_tag_close( context->dump, tagname );
}

static void dump_tupledesc( FunctionDumpContext * context, TupleDesc tupdesc )
{
  // FIXME incomplete
  xml_tag_open( context->dump, "TupleDesc" );
  xml_tag_close( context->dump, "TupleDesc" );
}

static void dump_exception( FunctionDumpContext * context, PLpgSQL_exception * datum )
{
  xml_tag_open( context->dump, "Exception" );
  // FIXME incomplete 
  xml_tag_close( context->dump, "Exception" );
}

