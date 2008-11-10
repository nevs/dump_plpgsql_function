
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
  bool retval;
  if (node == NULL) return false;

  const char *tagname = NodeTagNames[nodeTag(node)] ? : "node";

  xml_tag_open( context, tagname, NULL );

  switch( nodeTag(node) ) {
    case T_String:
      xml_content( context, strVal( node ) );
      break;
    case T_A_Const: 
      parse_tree_walker((Node *) &(((A_Const *)node)->val), (void *) context );
      break;
    case T_A_Expr:
      xml_tag_open( context, "Operator", "type", A_Expr_Kind_Names[((A_Expr *)node)->kind], NULL );
      switch( ((A_Expr *)node)->kind ) {
        case AEXPR_OP:
          xml_tag_open( context, "Name", NULL );
          parse_tree_walker((Node *) ((A_Expr *)node)->name, (void *) context );
          xml_tag_close( context, "Name" );
          break;
        default:
          break;
      }
      xml_tag_close( context, "Operator" );

      xml_tag_open( context, "Left", NULL );
      parse_tree_walker((Node *) ((A_Expr *)node)->lexpr, (void *) context );
      xml_tag_close( context, "Left" );

      xml_tag_open( context, "Right", NULL );
      parse_tree_walker((Node *) ((A_Expr *)node)->rexpr, (void *) context );
      xml_tag_close( context, "Right" );
      break;
    case T_A_Indirection:
      parse_tree_walker((Node *) ((A_Indirection *)node)->arg, (void *) context );

    default: 
      retval = raw_expression_tree_walker(node, parse_tree_walker, (void *) context );
      break;
  }

  xml_tag_close( context, tagname );

  return retval;
}

static void dump_parse_node( DumpContext dump, Node * node ) {
  ListCell * item;
  switch( nodeTag( node ) ) {
    case T_SelectStmt:
      append_string( dump.output, "<select>" );
      SelectStmt * select = (SelectStmt *) node;
      printf("setop: %d left: %p right: %p\n", select->op, select->larg, select->rarg );
      printf("where: %p from: %p into: %p\n", select->whereClause, select->fromClause, select->intoClause );
      append_string( dump.output, "<targets>" );
      foreach( item, select->targetList)
        dump_parse_node( dump, lfirst( item ) );
      append_string( dump.output, "</targets>" );
      append_string( dump.output, "</select>" );
      break;
/*
    case T_A_Indirection:
      append_string( dump.output, "<A_Indirection>", nodeTag( node ) );
      append_string( dump.output, "<arg>" );
      dump_parse_node( dump, ((A_Indirection * )node)->arg);
      append_string( dump.output, "</arg>" );
      append_string( dump.output, "</A_Indirection>", nodeTag( node ) );
      break;
*/
    case T_ResTarget:
      append_string( dump.output, "<result_target>" );
      ResTarget * target = (ResTarget *) node;
      if (target->name)
        append_string( dump.output, "<name>%s</name>", target->name );
      append_string( dump.output, "<value>" );
      dump_parse_node( dump, target->val );
      append_string( dump.output, "</value>" );
      if (target->indirection) {
        append_string( dump.output, "<indirection>" );
        foreach( item, target->indirection )
          dump_parse_node( dump, lfirst( item ) );
        append_string( dump.output, "</indirection>" );
      }
      append_string( dump.output, "</result_target>" );
      break;
    default:
      break;
  }
}

