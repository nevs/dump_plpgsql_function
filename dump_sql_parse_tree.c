
#include <postgres.h>
#include <parser/parser.h>
#include <nodes/nodeFuncs.h>

#include "string_helper.h"
#include "dump_sql_parse_tree.h"
#include "sql_parsetree_names.h"

#define CHILD_NODE( type, child ) conditional_child_node( ((type *)node)->child, context, #child)


bool parse_tree_walker( Node *node, DumpContext * context );

const char * dump_sql_parse_tree_internal( const char * query ) {
  DumpContext * context = palloc0( sizeof( DumpContext ) );
  context->output = palloc0( sizeof( char *));

  List * parsetree_list = raw_parser( query );
  raw_expression_tree_walker( (Node *) parsetree_list, parse_tree_walker, context );

  return *context->output;
}

bool conditional_child_node( Node * node, DumpContext * context, const char * tagname )
{
  bool retval = false;

  if ( node ) {
    xml_tag_open( context, tagname, NULL );
    retval = parse_tree_walker( node, (void *) context );
    xml_tag_close( context, tagname );
  }

  return retval;
}

bool parse_tree_walker( Node *node, DumpContext * context )
{
  bool retval = false;

  if (node == NULL) {
    xml_tag( context, "NULL", NULL );
    return false;
  }

  const char *tagname = NodeTagNames[nodeTag(node)] ? : "node";

  switch( nodeTag(node) ) {
    case T_Integer:
      xml_tag( context, tagname, "value", "%d", intVal( node ), NULL );
      break;
    case T_String:
      xml_tag( context, tagname, "value", "%s", strVal( node ), NULL );
      break;
    default:

      xml_tag_open( context, tagname, NULL );

      switch( nodeTag(node) ) {
        case T_SelectStmt:
          CHILD_NODE( SelectStmt, intoClause );
          CHILD_NODE( SelectStmt, distinctClause );
          CHILD_NODE( SelectStmt, havingClause );
          CHILD_NODE( SelectStmt, groupClause );
          CHILD_NODE( SelectStmt, withClause );
          CHILD_NODE( SelectStmt, fromClause );
          CHILD_NODE( SelectStmt, whereClause );
          CHILD_NODE( SelectStmt, targetList );
          break;
        case T_ResTarget:
          if (((ResTarget *)node)->name) {
            xml_tag( context, "name", "value", "%s", ((ResTarget *)node)->name, NULL );
          }
          CHILD_NODE( ResTarget, indirection );
          CHILD_NODE( ResTarget, val );

          break;
          
        case T_A_Const: 
          parse_tree_walker((Node *) &(((A_Const *)node)->val), (void *) context );
          break;
        case T_FuncCall:
          CHILD_NODE( FuncCall, funcname );
          CHILD_NODE( FuncCall, args );

          break;
        case T_ColumnRef:
          CHILD_NODE( ColumnRef, fields );
          break;
        case T_A_Expr:
          xml_tag_open( context, "operator", "type", A_Expr_Kind_Names[((A_Expr *)node)->kind], NULL );
          CHILD_NODE( A_Expr, name );
          xml_tag_close( context, "operator" );

          CHILD_NODE( A_Expr, lexpr );
          CHILD_NODE( A_Expr, rexpr );
          break;
        case T_A_Indirection:
          parse_tree_walker((Node *) ((A_Indirection *)node)->arg, (void *) context );

        default: 
          retval = raw_expression_tree_walker(node, parse_tree_walker, (void *) context );
          break;
      }

      xml_tag_close( context, tagname );
  };

  return retval;
}

