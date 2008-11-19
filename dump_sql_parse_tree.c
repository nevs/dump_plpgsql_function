
#include <postgres.h>
#include <parser/parser.h>
#include <nodes/nodeFuncs.h>

#include "string_helper.h"
#include "dump_sql_parse_tree.h"
#include "sql_parsetree_names.h"

extern const char * debug_query_string;

#define CHILD_NODE( type, child ) conditional_child_node( (Node *)((type *)node)->child, context, #child)
#define TEXT_NODE( type, child ) xml_textnode( context, #child, "%s", (char *)((type *)node)->child ? : "" )

bool parse_tree_walker( Node *node, DumpContext * context );

void dump_sql_parse_tree_internal( DumpContext * context, const char * query ) {
  List * parsetree_list = raw_parser( query );
  raw_expression_tree_walker( (Node *) parsetree_list, parse_tree_walker, context );
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
    case T_Integer:            // 651
      xml_tag( context, tagname, "value", "%d", intVal( node ), NULL );
      break;
    case T_String:             // 653
      xml_tag( context, tagname, "value", "%s", strVal( node ), NULL );
      break;
    default:

      xml_tag_open( context, tagname, NULL );

      switch( nodeTag(node) ) {
        case T_Alias:          // 300
          TEXT_NODE( Alias, aliasname );
          CHILD_NODE( Alias, colnames );
          break;
        case T_RangeVar:       // 301
          TEXT_NODE( RangeVar, catalogname );
          TEXT_NODE( RangeVar, schemaname );
          TEXT_NODE( RangeVar, relname );
          CHILD_NODE( RangeVar, alias );
          break;
        case T_JoinExpr:       // 340
          xml_textnode( context, "jointype", "%s", JoinTypeNames[((JoinExpr *)node)->jointype] );
          CHILD_NODE( JoinExpr, larg );
          CHILD_NODE( JoinExpr, rarg );
          CHILD_NODE( JoinExpr, using );
          CHILD_NODE( JoinExpr, quals );
          CHILD_NODE( JoinExpr, alias );
          break;
        case T_A_Expr:         // 900
          xml_tag_open( context, "operator", "type", A_Expr_Kind_Names[((A_Expr *)node)->kind], NULL );
          CHILD_NODE( A_Expr, name );
          xml_tag_close( context, "operator" );
          CHILD_NODE( A_Expr, lexpr );
          CHILD_NODE( A_Expr, rexpr );
          break;
        case T_ColumnRef:      // 901
          CHILD_NODE( ColumnRef, fields );
          break;
        case T_A_Const:        // 903
          parse_tree_walker((Node *) &(((A_Const *)node)->val), (void *) context );
          break;
        case T_FuncCall:       // 904
          CHILD_NODE( FuncCall, funcname );
          CHILD_NODE( FuncCall, args );
          break;
        case T_A_Indirection:  // 907
          CHILD_NODE( A_Indirection, arg );
          CHILD_NODE( A_Indirection, indirection );
          break;
        case T_ResTarget:      // 909
          TEXT_NODE( ResTarget, name );
          CHILD_NODE( ResTarget, indirection );
          CHILD_NODE( ResTarget, val );
          break;
        case T_SelectStmt:     // 705
          CHILD_NODE( SelectStmt, distinctClause );
          CHILD_NODE( SelectStmt, intoClause );
          CHILD_NODE( SelectStmt, targetList );
          CHILD_NODE( SelectStmt, fromClause );
          CHILD_NODE( SelectStmt, whereClause );
          CHILD_NODE( SelectStmt, groupClause );
          CHILD_NODE( SelectStmt, havingClause );
          CHILD_NODE( SelectStmt, withClause );
          CHILD_NODE( SelectStmt, valuesLists );
          CHILD_NODE( SelectStmt, sortClause );
          CHILD_NODE( SelectStmt, limitOffset );
          CHILD_NODE( SelectStmt, limitCount );
          CHILD_NODE( SelectStmt, lockingClause );

          if ( ((SelectStmt *)node)->op != SETOP_NONE ) {
            xml_tag( context, "op", "value", "%s", SetOperationNames[((SelectStmt*)node)->op], NULL );
            CHILD_NODE( SelectStmt, larg );
            CHILD_NODE( SelectStmt, rarg );
          }
          break;
        default: 
          retval = raw_expression_tree_walker(node, parse_tree_walker, (void *) context );
          break;
      }

      xml_tag_close( context, tagname );
  };

  return retval;
}

