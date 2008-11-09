
#define _GNU_SOURCE
#include <stdio.h>

#include <postgres.h>
#include <fmgr.h>
#include <parser/parse_type.h>
#include <nodes/nodeFuncs.h>
#include <plpgsql.h>
#include <lib/stringinfo.h>

#include "dump_plpgsql_function.h"
#include "sql_nodetag_names.h"


typedef struct dump_context {
  PLpgSQL_function * func;
  char ** output;
} DumpContext;

static void dump_sql_query( DumpContext* dump, const char * query_string, Oid *paramTypes, int numParams );

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(dump_sql_parse_tree);

Datum dump_sql_parse_tree( PG_FUNCTION_ARGS )
{
  DumpContext context;
  char * output = NULL;
  context.output = &output;

  const char * input = PG_GETARG_TEXT_P( 0 )->vl_dat;

  dump_sql_query( &context, input, NULL, 0 );

  if ( *context.output )
    PG_RETURN_TEXT_P( cstring_to_text( *context.output ) );
  else 
    PG_RETURN_NULL();
}

bool parse_tree_walker( Node *node, DumpContext * context )
{
  if (node == NULL) return false;

  const char *tagname = NodeTagNames[nodeTag(node)];

  if (tagname) {
    append_string( context->output, "<%s>", tagname );
  } else {
    tagname = "node";
    append_string( context->output, "<%s tag=\"%d\">", tagname, nodeTag( node ) );
  }

  switch( nodeTag(node) ) {
    case T_String:
      append_string( context->output, "%s", strVal( node ) );
      break;
    case T_A_Const: 
      parse_tree_walker((Node *) &(((A_Const *)node)->val), (void *) context );
      break;
    case T_A_Expr:
      append_string( context->output, "<operator type=\"%s\">", A_Expr_Kind_Names[((A_Expr *)node)->kind] );
      switch( ((A_Expr *)node)->kind ) {
        case AEXPR_OP:
          append_string( context->output, "<name>" );
          parse_tree_walker((Node *) ((A_Expr *)node)->name, (void *) context );
          append_string( context->output, "</name>" );
          break;
        default:
          break;
      }
      append_string( context->output, "</operator>" );

      append_string( context->output, "<left>" );
      parse_tree_walker((Node *) ((A_Expr *)node)->lexpr, (void *) context );
      append_string( context->output, "</left>" );

      append_string( context->output, "<right>" );
      parse_tree_walker((Node *) ((A_Expr *)node)->rexpr, (void *) context );
      append_string( context->output, "</right>" );
      break;
    case T_A_Indirection:
      parse_tree_walker((Node *) ((A_Indirection *)node)->arg, (void *) context );

    default: 
      raw_expression_tree_walker(node, parse_tree_walker, (void *) context );
      break;
  }

  append_string( context->output, "</%s>", tagname );

  return false;
}

static void dump_sql_query( DumpContext* context, const char * query_string, Oid *paramTypes, int numParams )
{
  List * parsetree_list = pg_parse_query(query_string);
  raw_expression_tree_walker( (Node *) parsetree_list, parse_tree_walker, (void  *) context );
}
