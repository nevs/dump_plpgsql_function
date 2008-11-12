
#include <postgres.h>
#include <parser/parser.h>
#include <nodes/nodeFuncs.h>

#include "string_helper.h"
#include "dump_sql_parse_tree.h"
#include "sql_parsetree_names.h"


bool parse_tree_walker( Node *node, DumpContext * context );

const char * dump_sql_parse_tree_internal( const char * query ) {
  DumpContext * context = palloc0( sizeof( DumpContext ) );
  context->output = palloc0( sizeof( char *));

  List * parsetree_list = raw_parser( query );
  raw_expression_tree_walker( (Node *) parsetree_list, parse_tree_walker, context );

  return *context->output;
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
          if (((SelectStmt *)node)->intoClause) {
            xml_tag_open( context, "into_clause", NULL );
            parse_tree_walker((Node *) ((SelectStmt *)node)->intoClause, (void *) context );
            xml_tag_close( context, "into_clause" );
          }

          if (((SelectStmt *)node)->distinctClause) {
            xml_tag_open( context, "distinct_clause", NULL );
            parse_tree_walker((Node *) ((SelectStmt *)node)->distinctClause, (void *) context );
            xml_tag_close( context, "distinct_clause" );
          }

          if (((SelectStmt *)node)->targetList) {
            xml_tag_open( context, "targets", NULL );
            parse_tree_walker((Node *) ((SelectStmt *)node)->targetList, (void *) context );
            xml_tag_close( context, "targets" );
          }

          if (((SelectStmt *)node)->havingClause) {
            xml_tag_open( context, "having_clause", NULL );
            parse_tree_walker((Node *) ((SelectStmt *)node)->havingClause, (void *) context );
            xml_tag_close( context, "having_clause" );
          }

          if (((SelectStmt *)node)->withClause) {
            xml_tag_open( context, "with_clause", NULL );
            parse_tree_walker((Node *) ((SelectStmt *)node)->withClause, (void *) context );
            xml_tag_close( context, "with_clause" );
          }

          if (((SelectStmt *)node)->fromClause) {
            xml_tag_open( context, "from_clause", NULL );
            parse_tree_walker((Node *) ((SelectStmt *)node)->fromClause, (void *) context );
            xml_tag_close( context, "from_clause" );
          }

          if (((SelectStmt *)node)->whereClause) {
            xml_tag_open( context, "where_clause", NULL );
            parse_tree_walker((Node *) ((SelectStmt *)node)->whereClause, (void *) context );
            xml_tag_close( context, "where_clause" );
          }

          break;
        case T_ResTarget:
          if (((ResTarget *)node)->name) {
            xml_tag( context, "name", "value", "%s", ((ResTarget *)node)->name, NULL );
          }

          if (((ResTarget *)node)->indirection) {
            xml_tag_open( context, "indirection", NULL );
            parse_tree_walker((Node *) ((ResTarget *)node)->indirection, (void *) context );
            xml_tag_close( context, "indirection" );
          }

          if (((ResTarget *)node)->val) {
            xml_tag_open( context, "value", NULL );
            parse_tree_walker((Node *) ((ResTarget *)node)->val, (void *) context );
            xml_tag_close( context, "value" );
          }
          break;
          
        case T_A_Const: 
          parse_tree_walker((Node *) &(((A_Const *)node)->val), (void *) context );
          break;
        case T_FuncCall:
          xml_tag_open( context, "function_name", NULL );
          parse_tree_walker((Node *) ((FuncCall *)node)->funcname, (void *) context );
          xml_tag_close( context, "function_name" );

          xml_tag_open( context, "arguments", NULL );
          parse_tree_walker((Node *) ((FuncCall *)node)->args, (void *) context );
          xml_tag_close( context, "arguments" );

          break;
        case T_ColumnRef:
          xml_tag_open( context, "fields", NULL );
          parse_tree_walker((Node *) ((ColumnRef *)node)->fields, (void *) context );
          xml_tag_close( context, "fields" );
          break;
        case T_A_Expr:
          xml_tag_open( context, "operator", "type", A_Expr_Kind_Names[((A_Expr *)node)->kind], NULL );
          switch( ((A_Expr *)node)->kind ) {
            case AEXPR_OP:
              xml_tag_open( context, "name", NULL );
              parse_tree_walker((Node *) ((A_Expr *)node)->name, (void *) context );
              xml_tag_close( context, "name" );
              break;
            default:
              break;
          }
          xml_tag_close( context, "operator" );

          xml_tag_open( context, "left", NULL );
          parse_tree_walker((Node *) ((A_Expr *)node)->lexpr, (void *) context );
          xml_tag_close( context, "left" );

          xml_tag_open( context, "right", NULL );
          parse_tree_walker((Node *) ((A_Expr *)node)->rexpr, (void *) context );
          xml_tag_close( context, "right" );
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

