
#include <postgres.h>
#include <parser/parser.h>
#include <nodes/nodeFuncs.h>

#include "string_helper.h"
#include "dump_sql_parse_tree.h"
#include "sql_parsetree_names.h"

extern const char * debug_query_string;

#define CHILD_NODE( type, child ) conditional_child_node( (Node *)((type *)node)->child, context, #child)
#define TEXT_NODE( type, child ) xml_textnode( context, #child, "%s", (char *)((type *)node)->child ? : "" )
#define CHAR_NODE( type, child ) xml_textnode( context, #child, "%c", ((type *)node)->child )
#define NUMBER_NODE( type, child ) xml_textnode( context, #child, "%d", ((type *)node)->child )
#define LONG_NODE( type, child ) xml_textnode( context, #child, "%ld", ((type *)node)->child )
#define DOUBLE_NODE( type, child ) xml_textnode( context, #child, "%f", ((type *)node)->child )
#define BOOL_NODE( type, child )  xml_textnode( context, #child, "%d", ((type *)node)->child )

#define DICT_NODE( type, child, values ) xml_textnode( context, #child, "%s", values[((type *)node)->child] )

bool parse_tree_walker( Node *node, DumpContext * context );

void dump_sql_parse_tree_internal( DumpContext * context, const char * query ) {
  xml_tag_open_namespace( context, "sql_parse_tree", "sql:" );
  List * parsetree_list = raw_parser( query );
  raw_expression_tree_walker( (Node *) parsetree_list, parse_tree_walker, context );
  xml_tag_close( context, "sql_parse_tree" );
}

bool conditional_child_node( Node * node, DumpContext * context, const char * tagname )
{
  bool retval = false;

  if ( node ) {
    xml_tag_open( context, tagname );
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

  const char *tagname = NodeTag_Names[nodeTag(node)] ? : "node";

  xml_tag_open( context, tagname );

  switch( nodeTag(node) ) {
    case T_Alias:          // 300
      TEXT_NODE( Alias, aliasname );
      CHILD_NODE( Alias, colnames );
      break;
    case T_RangeVar:
      TEXT_NODE( RangeVar, catalogname );
      TEXT_NODE( RangeVar, schemaname );
      TEXT_NODE( RangeVar, relname );
      CHILD_NODE( RangeVar, alias );
      break;
    case T_Expr:
      break;
    case T_Var:
      NUMBER_NODE( Var, varno );
      NUMBER_NODE( Var, varattno );
      NUMBER_NODE( Var, vartype );
      NUMBER_NODE( Var, vartypmod  );
      NUMBER_NODE( Var, varlevelsup );
      NUMBER_NODE( Var, varnoold );
      NUMBER_NODE( Var, varoattno );
      NUMBER_NODE( Var, location );
      break;
    case T_Const:
      NUMBER_NODE( Const, consttype );
      NUMBER_NODE( Const, consttypmod );
      NUMBER_NODE( Const, constlen);
      CHILD_NODE( Const, constvalue );
      BOOL_NODE( Const, constisnull );
      BOOL_NODE( Const, constbyval );
      NUMBER_NODE( Var, location );
      break;
    case T_Param:
      DICT_NODE( Param, paramkind, ParamKind_Names );
      NUMBER_NODE( Param, paramid );
      NUMBER_NODE( Param, paramtype );
      NUMBER_NODE( Param, paramtypmod );
      NUMBER_NODE( Param, location );
      break;
    case T_Aggref:
      NUMBER_NODE( Aggref, aggfnoid );
      NUMBER_NODE( Aggref, aggtype );
      CHILD_NODE( Aggref, args );
      NUMBER_NODE( Aggref, agglevelsup );
      BOOL_NODE( Aggref, aggstar );
      BOOL_NODE( Aggref, aggdistinct );
      NUMBER_NODE( Aggref, location );
      break;
    case T_ArrayRef:
      NUMBER_NODE( ArrayRef, refarraytype );
      NUMBER_NODE( ArrayRef, refelemtype );
      NUMBER_NODE( ArrayRef, reftypmod );
      CHILD_NODE( ArrayRef, refupperindexpr );
      CHILD_NODE( ArrayRef, reflowerindexpr );
      CHILD_NODE( ArrayRef, refexpr );
      CHILD_NODE( ArrayRef, refassgnexpr );
      break;
    case T_FuncExpr:
      NUMBER_NODE( FuncExpr, funcid );
      NUMBER_NODE( FuncExpr, funcresulttype );
      BOOL_NODE( FuncExpr, funcretset );
      DICT_NODE( FuncExpr, funcformat, CoercionForm_Names );
      CHILD_NODE( FuncExpr, args );
      NUMBER_NODE( FuncExpr, location );
      break;
    case T_OpExpr:
    case T_DistinctExpr:
    case T_NullIfExpr:
      NUMBER_NODE( OpExpr, opno );
      NUMBER_NODE( OpExpr, opfuncid );
      NUMBER_NODE( OpExpr, opresulttype );
      BOOL_NODE( OpExpr, opretset );
      CHILD_NODE( OpExpr, args );
      NUMBER_NODE( OpExpr, location );
      break;
    case T_ScalarArrayOpExpr:
      NUMBER_NODE( ScalarArrayOpExpr, opno );
      NUMBER_NODE( ScalarArrayOpExpr, opfuncid );
      BOOL_NODE( ScalarArrayOpExpr, useOr );
      CHILD_NODE( ScalarArrayOpExpr, args );
      NUMBER_NODE( ScalarArrayOpExpr, location );
      break;
    case T_BoolExpr:
      DICT_NODE( BoolExpr, boolop, BoolExprType_Names );
      CHILD_NODE( BoolExpr, args );
      NUMBER_NODE( BoolExpr, location );
      break;
    case T_SubLink:
      DICT_NODE( SubLink, subLinkType, SubLinkType_Names );
      CHILD_NODE( SubLink, testexpr );
      CHILD_NODE( SubLink, operName );
      CHILD_NODE( SubLink, subselect );
      NUMBER_NODE( SubLink, location );
      break;
    case T_SubPlan:
      DICT_NODE( SubPlan, subLinkType, SubLinkType_Names );
      CHILD_NODE( SubPlan, testexpr );
      CHILD_NODE( SubPlan, paramIds );
      NUMBER_NODE( SubPlan, plan_id );
      TEXT_NODE( SubPlan, plan_name );
      NUMBER_NODE( SubPlan, firstColType );
      NUMBER_NODE( SubPlan, firstColTypmod );
      BOOL_NODE( SubPlan, useHashTable );
      BOOL_NODE( SubPlan, unknownEqFalse );
      CHILD_NODE( SubPlan, setParam );
      CHILD_NODE( SubPlan, parParam );
      CHILD_NODE( SubPlan, args );
      DOUBLE_NODE( SubPlan, startup_cost );
      DOUBLE_NODE( SubPlan, per_call_cost );
      break;
    case T_AlternativeSubPlan:
      CHILD_NODE( AlternativeSubPlan, subplans );
      break;
    case T_FieldSelect:
      CHILD_NODE( FieldSelect, arg );
      NUMBER_NODE( FieldSelect, fieldnum );
      NUMBER_NODE( FieldSelect, resulttype );
      NUMBER_NODE( FieldSelect, resulttypmod );
      break;
    case T_FieldStore:
      CHILD_NODE( FieldStore, arg );
      CHILD_NODE( FieldStore, newvals );
      CHILD_NODE( FieldStore, fieldnums );
      NUMBER_NODE( FieldStore, resulttype );
      break;
    case T_RelabelType:
      CHILD_NODE( RelabelType, arg );
      NUMBER_NODE( RelabelType, resulttype );
      NUMBER_NODE( RelabelType, resulttypmod );
      DICT_NODE( RelabelType, relabelformat, CoercionForm_Names );
      NUMBER_NODE( RelabelType, location );
      break;
    case T_CoerceViaIO:
      CHILD_NODE( CoerceViaIO, arg );
      NUMBER_NODE( CoerceViaIO, resulttype );
      DICT_NODE( CoerceViaIO, coerceformat, CoercionForm_Names );
      NUMBER_NODE( CoerceViaIO, location );
      break;
    case T_ArrayCoerceExpr:
      CHILD_NODE( ArrayCoerceExpr, arg );
      NUMBER_NODE( ArrayCoerceExpr, elemfuncid );
      NUMBER_NODE( ArrayCoerceExpr, resulttype );
      NUMBER_NODE( ArrayCoerceExpr, resulttypmod );
      BOOL_NODE( ArrayCoerceExpr, isExplicit );
      DICT_NODE( ArrayCoerceExpr, coerceformat, CoercionForm_Names );
      NUMBER_NODE( ArrayCoerceExpr, location );
      break;
    case T_ConvertRowtypeExpr:
      CHILD_NODE( ConvertRowtypeExpr, arg );
      NUMBER_NODE( ConvertRowtypeExpr, resulttype );
      DICT_NODE( ConvertRowtypeExpr, convertformat, CoercionForm_Names );
      NUMBER_NODE( ConvertRowtypeExpr, location );
      break;
    case T_CaseExpr:
      NUMBER_NODE( CaseExpr, casetype );
      CHILD_NODE( CaseExpr, arg );
      CHILD_NODE( CaseExpr, args );
      CHILD_NODE( CaseExpr, defresult );
      NUMBER_NODE( CaseExpr, location );
      break;
    case T_CaseWhen:
      CHILD_NODE( CaseWhen, expr );
      CHILD_NODE( CaseWhen, result );
      NUMBER_NODE( CaseWhen, location );
      break;
    case T_CaseTestExpr:
      NUMBER_NODE( CaseTestExpr, typeId );
      NUMBER_NODE( CaseTestExpr, typeMod );
      break;
    case T_ArrayExpr:
      NUMBER_NODE( ArrayExpr, array_typeid );
      NUMBER_NODE( ArrayExpr, element_typeid );
      CHILD_NODE( ArrayExpr, elements );
      BOOL_NODE( ArrayExpr, multidims );
      NUMBER_NODE( ArrayExpr, location );
      break;
    case T_RowExpr:
      CHILD_NODE( RowExpr, args );
      NUMBER_NODE( RowExpr, row_typeid );
      DICT_NODE( RowExpr, row_format, CoercionForm_Names );
      CHILD_NODE( RowExpr, colnames );
      NUMBER_NODE( RowExpr, location );
      break;
    case T_RowCompareExpr:
      DICT_NODE( RowCompareExpr, rctype, RowCompareType_Names );
      CHILD_NODE( RowCompareExpr, opnos );
      CHILD_NODE( RowCompareExpr, opfamilies );
      CHILD_NODE( RowCompareExpr, largs );
      CHILD_NODE( RowCompareExpr, rargs );
      break;
    case T_CoalesceExpr:
      NUMBER_NODE( CoalesceExpr, coalescetype );
      CHILD_NODE( CoalesceExpr, args );
      NUMBER_NODE( CoalesceExpr, location );
      break;
    case T_MinMaxExpr:
      NUMBER_NODE( MinMaxExpr, minmaxtype );
      DICT_NODE( MinMaxExpr, op, MinMaxOp_Names );
      CHILD_NODE( MinMaxExpr, args );
      NUMBER_NODE( MinMaxExpr, location );
      break;
    case T_XmlExpr:
      DICT_NODE( XmlExpr, op, XmlExprOp_Names );
      TEXT_NODE( XmlExpr, name );
      CHILD_NODE( XmlExpr, named_args );
      CHILD_NODE( XmlExpr, arg_names );
      CHILD_NODE( XmlExpr, args );
      DICT_NODE( XmlExpr, xmloption, XmlOptionType_Names );
      NUMBER_NODE( XmlExpr, type );
      NUMBER_NODE( XmlExpr, typmod );
      NUMBER_NODE( XmlExpr, location );
      break;
    case T_NullTest:
      CHILD_NODE( NullTest, arg );
      DICT_NODE( NullTest, nulltesttype, NullTestType_Names );
      break;
    case T_BooleanTest:
      CHILD_NODE( BooleanTest, arg );
      DICT_NODE( BooleanTest, booltesttype, BoolTestType_Names );
      break;
    case T_CoerceToDomain:
      CHILD_NODE( CoerceToDomain, arg );
      NUMBER_NODE( CoerceToDomain, resulttype );
      NUMBER_NODE( CoerceToDomain, resulttypmod );
      DICT_NODE( CoerceToDomain, coercionformat, CoercionForm_Names );
      NUMBER_NODE( CoerceToDomain, location );
      break;
    case T_CoerceToDomainValue:
      NUMBER_NODE( CoerceToDomainValue, typeId );
      NUMBER_NODE( CoerceToDomainValue, typeMod );
      NUMBER_NODE( CoerceToDomainValue, location );
      break;
    case T_SetToDefault:
      NUMBER_NODE( SetToDefault, typeId );
      NUMBER_NODE( SetToDefault, typeMod );
      NUMBER_NODE( SetToDefault, location );
      break;
    case T_CurrentOfExpr:
      NUMBER_NODE( CurrentOfExpr, cvarno );
      TEXT_NODE( CurrentOfExpr, cursor_name );
      NUMBER_NODE( CurrentOfExpr, cursor_param );
      break;
    case T_TargetEntry:
      CHILD_NODE( TargetEntry, expr );
      NUMBER_NODE( TargetEntry, resno );
      TEXT_NODE( TargetEntry, resname );
      NUMBER_NODE( TargetEntry, ressortgroupref );
      NUMBER_NODE( TargetEntry, resorigtbl );
      NUMBER_NODE( TargetEntry, resorigcol );
      BOOL_NODE( TargetEntry, resjunk );
      break;
    case T_RangeTblRef:
      NUMBER_NODE( RangeTblRef, rtindex );
      break;
    case T_JoinExpr:       // 340
      DICT_NODE( JoinExpr, jointype, JoinType_Names );
      BOOL_NODE( JoinExpr, isNatural );
      CHILD_NODE( JoinExpr, larg );
      CHILD_NODE( JoinExpr, rarg );
      CHILD_NODE( JoinExpr, using );
      CHILD_NODE( JoinExpr, quals );
      CHILD_NODE( JoinExpr, alias );
      NUMBER_NODE( JoinExpr, rtindex );
      break;
    case T_FromExpr:       // 341
      CHILD_NODE( FromExpr, fromlist );
      CHILD_NODE( FromExpr, quals );
      break;
    case T_IntoClause:     // 342
      CHILD_NODE( IntoClause, rel );
      CHILD_NODE( IntoClause, colNames );
      CHILD_NODE( IntoClause, options );
      DICT_NODE( IntoClause, onCommit, OnCommitAction_Names );
      TEXT_NODE( IntoClause, tableSpaceName );
      break;
    case T_Integer:        // 651
      xml_textnode( context, "value", "%ld", intVal( node ), NULL );
      break;
    case T_Float:          // 652
      xml_textnode( context, "value", "%f", floatVal( node ), NULL );
      break;
    case T_String:         // 653
      xml_textnode( context, "value", "%s", strVal( node ), NULL );
      break;
    case T_BitString:      // 654
    case T_Null:           // 655
      break;
    case T_IntList:
    case T_OidList:
    case T_List:
      retval = raw_expression_tree_walker(node, parse_tree_walker, (void *) context );
      break;
    case T_Query:          // 700
      DICT_NODE( Query, commandType, CmdType_Names );
      DICT_NODE( Query, querySource, QuerySource_Names );
      BOOL_NODE( Query, canSetTag );
      CHILD_NODE( Query, utilityStmt );
      NUMBER_NODE( Query, resultRelation );
      CHILD_NODE( Query, intoClause );
      BOOL_NODE( Query, hasAggs );
      BOOL_NODE( Query, hasWindowFuncs );
      BOOL_NODE( Query, hasSubLinks );
      BOOL_NODE( Query, hasDistinctOn );
      BOOL_NODE( Query, hasRecursive );
      CHILD_NODE( Query, cteList );
      CHILD_NODE( Query, rtable );
      CHILD_NODE( Query, jointree );
      CHILD_NODE( Query, targetList );
      CHILD_NODE( Query, returningList );
      CHILD_NODE( Query, groupClause );
      CHILD_NODE( Query, havingQual );
      CHILD_NODE( Query, windowClause );
      CHILD_NODE( Query, distinctClause );
      CHILD_NODE( Query, sortClause );
      CHILD_NODE( Query, limitOffset );
      CHILD_NODE( Query, limitCount );
      CHILD_NODE( Query, rowMarks );
      CHILD_NODE( Query, setOperations );
      break;
    case T_InsertStmt:     // 702
      CHILD_NODE( InsertStmt, relation );
      CHILD_NODE( InsertStmt, cols );
      CHILD_NODE( InsertStmt, selectStmt );
      CHILD_NODE( InsertStmt, returningList );
      break;
    case T_DeleteStmt:     // 703
      CHILD_NODE( DeleteStmt, relation );
      CHILD_NODE( DeleteStmt, usingClause );
      CHILD_NODE( DeleteStmt, whereClause );
      CHILD_NODE( DeleteStmt, returningList );
      break;
    case T_UpdateStmt:     // 704
      CHILD_NODE( UpdateStmt, relation );
      CHILD_NODE( UpdateStmt, targetList );
      CHILD_NODE( UpdateStmt, whereClause );
      CHILD_NODE( UpdateStmt, fromClause );
      CHILD_NODE( UpdateStmt, returningList );
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
        DICT_NODE( SelectStmt, op, SetOperation_Names );
        CHILD_NODE( SelectStmt, larg );
        CHILD_NODE( SelectStmt, rarg );
      }
      break;
    case T_AlterTableStmt:
      CHILD_NODE( AlterTableStmt, relation );
      CHILD_NODE( AlterTableStmt, cmds );
      DICT_NODE( AlterTableStmt, relkind, ObjectType_Names );
      break;
    case T_AlterTableCmd:
      DICT_NODE( AlterTableCmd, subtype, AlterTableType_Names );
      TEXT_NODE( AlterTableCmd, name );
      CHILD_NODE( AlterTableCmd, def );
      CHILD_NODE( AlterTableCmd, transform );
      DICT_NODE( AlterTableCmd, behavior, DropBehavior_Names );
      break;
    case T_AlterDomainStmt:
      CHAR_NODE( AlterDomainStmt, subtype );
      CHILD_NODE( AlterDomainStmt, typename );
      TEXT_NODE( AlterDomainStmt, name );
      CHILD_NODE( AlterDomainStmt, def );
      DICT_NODE( AlterDomainStmt, behavior, DropBehavior_Names );
      break;
    case T_SetOperationStmt:
      DICT_NODE( SetOperationStmt, op, SetOperation_Names );
      BOOL_NODE( SetOperationStmt, all );
      CHILD_NODE( SetOperationStmt, larg );
      CHILD_NODE( SetOperationStmt, rarg );
      CHILD_NODE( SetOperationStmt, colTypes );
      CHILD_NODE( SetOperationStmt, colTypmods );
      CHILD_NODE( SetOperationStmt, groupClauses );
      break;
    case T_GrantStmt:
      BOOL_NODE( GrantStmt, is_grant );
      DICT_NODE( GrantStmt, objtype, GrantObjectType_Names );
      CHILD_NODE( GrantStmt, objects );
      CHILD_NODE( GrantStmt, privileges );
      CHILD_NODE( GrantStmt, grantees );
      BOOL_NODE( GrantStmt, grant_option );
      DICT_NODE( GrantStmt, behavior, DropBehavior_Names );
      break; 
    case T_GrantRoleStmt:
      CHILD_NODE( GrantRoleStmt, granted_roles );
      CHILD_NODE( GrantRoleStmt, grantee_roles );
      BOOL_NODE( GrantRoleStmt, is_grant );
      BOOL_NODE( GrantRoleStmt, admin_opt );
      TEXT_NODE( GrantRoleStmt, grantor );
      DICT_NODE( GrantRoleStmt, behavior, DropBehavior_Names );
      break; 
    case T_ClosePortalStmt:
      TEXT_NODE( ClosePortalStmt, portalname );
      break;
    case T_ClusterStmt:
      CHILD_NODE( ClusterStmt, relation );
      TEXT_NODE( ClusterStmt, indexname );
      BOOL_NODE( ClusterStmt, verbose );
      break;
    case T_CopyStmt:
      CHILD_NODE( CopyStmt, relation );
      CHILD_NODE( CopyStmt, query );
      CHILD_NODE( CopyStmt, attlist );
      BOOL_NODE( CopyStmt, is_from );
      TEXT_NODE( CopyStmt, filename );
      CHILD_NODE( CopyStmt, options );
      break;
    case T_CreateStmt:
      CHILD_NODE( CreateStmt, relation );
      CHILD_NODE( CreateStmt, tableElts );
      CHILD_NODE( CreateStmt, inhRelations );
      CHILD_NODE( CreateStmt, constraints );
      CHILD_NODE( CreateStmt, options );
      xml_textnode( context, "oncommit", "%s", OnCommitAction_Names[((CreateStmt*)node)->oncommit] );
      TEXT_NODE( CreateStmt, tablespacename );
      break;
    case T_DefineStmt:
      xml_textnode( context, "kind", "%s", ObjectType_Names[((DefineStmt *)node)->kind] );
      BOOL_NODE( DefineStmt, oldstyle );
      CHILD_NODE( DefineStmt, defnames );
      CHILD_NODE( DefineStmt, args );
      CHILD_NODE( DefineStmt, definition );
      break;
    case T_DropStmt:
      CHILD_NODE( DropStmt, objects );
      xml_textnode( context, "removeType", "%s", ObjectType_Names[((DropStmt*)node)->removeType] );
      xml_textnode( context, "behavior", "%s", DropBehavior_Names[((DropStmt *)node)->behavior] );
      BOOL_NODE( DropStmt, missing_ok );
      break;
    case T_TruncateStmt:
      CHILD_NODE( TruncateStmt, relations );
      BOOL_NODE( TruncateStmt, restart_seqs );
      xml_textnode( context, "behavior", "%s", DropBehavior_Names[((TruncateStmt*)node)->behavior] );
      break;
    case T_CommentStmt:
      xml_textnode( context, "objtype", "%s", ObjectType_Names[((CommentStmt *)node)->objtype] );
      CHILD_NODE( CommentStmt, objname );
      CHILD_NODE( CommentStmt, objargs );
      TEXT_NODE( CommentStmt, comment );
      break;
    case T_FetchStmt:
      xml_textnode( context, "direction", "%s", FetchDirection_Names[((FetchStmt*)node)->direction] );
      LONG_NODE( FetchStmt, howMany );
      TEXT_NODE( FetchStmt, portalname );
      BOOL_NODE( FetchStmt, ismove );
      break;
    case T_IndexStmt:
      TEXT_NODE( IndexStmt, idxname );
      CHILD_NODE( IndexStmt, relation );
      TEXT_NODE( IndexStmt, accessMethod );
      TEXT_NODE( IndexStmt, tableSpace );
      CHILD_NODE( IndexStmt, indexParams );
      CHILD_NODE( IndexStmt, options );
      CHILD_NODE( IndexStmt, whereClause );
      BOOL_NODE( IndexStmt, unique );
      BOOL_NODE( IndexStmt, primary );
      BOOL_NODE( IndexStmt, isconstraint );
      BOOL_NODE( IndexStmt, concurrent );
      break;
    case T_CreateFunctionStmt:
      BOOL_NODE( CreateFunctionStmt, replace );
      CHILD_NODE( CreateFunctionStmt, funcname );
      CHILD_NODE( CreateFunctionStmt, parameters );
      CHILD_NODE( CreateFunctionStmt, returnType );
      CHILD_NODE( CreateFunctionStmt, options );
      CHILD_NODE( CreateFunctionStmt, withClause );
      break;
    case T_AlterFunctionStmt:
      CHILD_NODE( AlterFunctionStmt, func );
      CHILD_NODE( AlterFunctionStmt, actions );
      break;
    case T_RemoveFuncStmt:
      xml_textnode( context, "kind", "%s", ObjectType_Names[((RemoveFuncStmt *)node)->kind] );
      CHILD_NODE( RemoveFuncStmt, name );
      CHILD_NODE( RemoveFuncStmt, args );
      xml_textnode( context, "behavior", "%s", DropBehavior_Names[((RemoveFuncStmt *)node)->behavior] );
      BOOL_NODE( RemoveFuncStmt, missing_ok );
      break;
    case T_RenameStmt:
      xml_textnode( context, "renameType", "%s", ObjectType_Names[((RenameStmt *)node)->renameType] );
      CHILD_NODE( RenameStmt, relation );
      CHILD_NODE( RenameStmt, object );
      CHILD_NODE( RenameStmt, objarg );
      TEXT_NODE( RenameStmt, subname );
      TEXT_NODE( RenameStmt, newname );
      break;
    case T_RuleStmt:
      CHILD_NODE( RuleStmt, relation );
      TEXT_NODE( RuleStmt, rulename );
      CHILD_NODE( RuleStmt, whereClause );
      xml_textnode( context, "event", "%s", CmdType_Names[((RuleStmt *)node)->event] );
      BOOL_NODE( RuleStmt, instead );
      CHILD_NODE( RuleStmt, actions );
      BOOL_NODE( RuleStmt, replace );
      break;
    case T_NotifyStmt:
      TEXT_NODE( NotifyStmt, conditionname );
      break;
    case T_ListenStmt:
      TEXT_NODE( ListenStmt, conditionname );
      break;
    case T_UnlistenStmt:
      TEXT_NODE( UnlistenStmt, conditionname );
      break;
    case T_TransactionStmt:
      xml_textnode( context, "kind", "%s", TransactionStmtKind_Names[((TransactionStmt *)node)->kind] );
      CHILD_NODE( TransactionStmt, options );
      TEXT_NODE( TransactionStmt, gid );
      break;
    case T_ViewStmt:
      CHILD_NODE( ViewStmt, view );
      CHILD_NODE( ViewStmt, aliases );
      CHILD_NODE( ViewStmt, query );
      BOOL_NODE( ViewStmt, replace );
      break;
    case T_LoadStmt:
      TEXT_NODE( LoadStmt, filename );
      break;
    case T_CreateDomainStmt:
      CHILD_NODE( CreateDomainStmt, domainname );
      CHILD_NODE( CreateDomainStmt, typename );
      CHILD_NODE( CreateDomainStmt, constraints );
      break;
    case T_CreatedbStmt:
      TEXT_NODE( CreatedbStmt, dbname );
      CHILD_NODE( CreatedbStmt, options );
      break;
    case T_DropdbStmt:
      TEXT_NODE( DropdbStmt, dbname );
      BOOL_NODE( DropdbStmt, missing_ok );
      break;
    case T_VacuumStmt:
      BOOL_NODE( VacuumStmt, vacuum );
      BOOL_NODE( VacuumStmt, full );
      BOOL_NODE( VacuumStmt, analyze );
      BOOL_NODE( VacuumStmt, verbose );
      NUMBER_NODE( VacuumStmt, freeze_min_age );
      NUMBER_NODE( VacuumStmt, freeze_table_age );
      CHILD_NODE( VacuumStmt, relation );
      CHILD_NODE( VacuumStmt, va_cols );
      break;
    case T_ExplainStmt:
      CHILD_NODE( ExplainStmt, query );
      BOOL_NODE( ExplainStmt, verbose );
      BOOL_NODE( ExplainStmt, analyze );
      break;
    case T_CreateSeqStmt:
      CHILD_NODE( CreateSeqStmt, sequence );
      CHILD_NODE( CreateSeqStmt, options );
      break;
    case T_AlterSeqStmt:
      CHILD_NODE( AlterSeqStmt, sequence );
      CHILD_NODE( AlterSeqStmt, options );
      break;
    case T_VariableSetStmt:
      xml_textnode( context, "kind", "%s", VariableSetKind_Names[((VariableSetStmt *)node)->kind] );
      TEXT_NODE( VariableSetStmt, name );
      CHILD_NODE( VariableSetStmt, args );
      BOOL_NODE( VariableSetStmt, is_local );
      break;
    case T_VariableShowStmt:
      TEXT_NODE( VariableShowStmt, name );
      break;
    case T_DiscardStmt:
      xml_textnode( context, "target", "%s", DiscardMode_Names[((DiscardStmt *)node)->target] );
      break;
    case T_CreateTrigStmt:
      TEXT_NODE( CreateTrigStmt, trigname );
      CHILD_NODE( CreateTrigStmt, relation );
      CHILD_NODE( CreateTrigStmt, funcname );
      CHILD_NODE( CreateTrigStmt, args );
      BOOL_NODE( CreateTrigStmt, before );
      BOOL_NODE( CreateTrigStmt, row );
      NUMBER_NODE( CreateTrigStmt, events );
      BOOL_NODE( CreateTrigStmt, isconstraint );
      BOOL_NODE( CreateTrigStmt, deferrable );
      BOOL_NODE( CreateTrigStmt, initdeferred );
      CHILD_NODE( CreateTrigStmt, constrrel );
      break;
    case T_DropPropertyStmt:
      CHILD_NODE( DropPropertyStmt, relation );
      CHILD_NODE( DropPropertyStmt, property );
      xml_textnode( context, "removeType", "%s", ObjectType_Names[((DropPropertyStmt*)node)->removeType] );
      xml_textnode( context, "behavior", "%s", DropBehavior_Names[((DropPropertyStmt*)node)->behavior] );
      BOOL_NODE( DropPropertyStmt, missing_ok );
      break;
    case T_CreatePLangStmt:
      TEXT_NODE( CreatePLangStmt, plname );
      CHILD_NODE( CreatePLangStmt, plhandler );
      CHILD_NODE( CreatePLangStmt, plvalidator );
      BOOL_NODE( CreatePLangStmt, pltrusted );
      break;
    case T_DropPLangStmt:
      TEXT_NODE( DropPLangStmt, plname );
      xml_textnode( context, "behavior", "%s", DropBehavior_Names[((DropPLangStmt*)node)->behavior] );
      BOOL_NODE( DropPLangStmt, missing_ok );
      break;
    case T_CreateRoleStmt:
      xml_textnode( context, "stmt_type", "%s", RoleStmtType_Names[((CreateRoleStmt*)node)->stmt_type] );
      TEXT_NODE( CreateRoleStmt, role );
      CHILD_NODE( CreateRoleStmt, options );
      break;
    case T_AlterRoleStmt:
      TEXT_NODE( AlterRoleStmt, role ); 
      CHILD_NODE( AlterRoleStmt, options );
      NUMBER_NODE( AlterRoleStmt, action );
      break;
    case T_DropRoleStmt:
      CHILD_NODE( DropRoleStmt, roles );
      BOOL_NODE( DropRoleStmt, missing_ok );
      break;
    case T_LockStmt:
      CHILD_NODE( LockStmt, relations );
      NUMBER_NODE( LockStmt, mode );
      BOOL_NODE( LockStmt, nowait );
      break;
    case T_ConstraintsSetStmt:
      CHILD_NODE( ConstraintsSetStmt, constraints );
      BOOL_NODE( ConstraintsSetStmt, deferred );
      break;
    case T_ReindexStmt:
      xml_textnode( context, "kind", "%s", ObjectType_Names[((ReindexStmt*)node)->kind] );
      CHILD_NODE( ReindexStmt, relation );
      TEXT_NODE( ReindexStmt, name );
      BOOL_NODE( ReindexStmt, do_system );
      BOOL_NODE( ReindexStmt, do_user );
      break;
    case T_CheckPointStmt:
      break;
    case T_CreateSchemaStmt:
      TEXT_NODE( CreateSchemaStmt, schemaname );
      TEXT_NODE( CreateSchemaStmt, authid );
      CHILD_NODE( CreateSchemaStmt, schemaElts );
      break;
    case T_AlterDatabaseStmt:
      TEXT_NODE( AlterDatabaseStmt, dbname );
      CHILD_NODE( AlterDatabaseStmt, options );
      break;
    case T_AlterDatabaseSetStmt:
      TEXT_NODE( AlterDatabaseSetStmt, dbname );
      CHILD_NODE( AlterDatabaseSetStmt, setstmt );
      break;
    case T_AlterRoleSetStmt:
      TEXT_NODE( AlterRoleSetStmt, role );
      CHILD_NODE( AlterRoleSetStmt, setstmt );
      break;
    case T_CreateConversionStmt:
      CHILD_NODE( CreateConversionStmt, conversion_name );
      TEXT_NODE( CreateConversionStmt, for_encoding_name );
      TEXT_NODE( CreateConversionStmt, to_encoding_name );
      CHILD_NODE( CreateConversionStmt, func_name );
      BOOL_NODE( CreateConversionStmt, def );
      break;
    case T_CreateCastStmt:
      CHILD_NODE( CreateCastStmt, sourcetype );
      CHILD_NODE( CreateCastStmt, targettype );
      CHILD_NODE( CreateCastStmt, func );
      xml_textnode( context, "context", "%s", CoercionContext_Names[((CreateCastStmt*)node)->context] );
      BOOL_NODE( CreateCastStmt, inout );
      break;
    case T_DropCastStmt:
      CHILD_NODE( DropCastStmt, sourcetype );
      CHILD_NODE( DropCastStmt, targettype );
      xml_textnode( context, "behavior", "%s", DropBehavior_Names[((DropCastStmt*)node)->behavior] );
      BOOL_NODE( DropCastStmt, missing_ok );
      break;
    case T_CreateOpClassStmt:
      CHILD_NODE( CreateOpClassStmt, opclassname );
      CHILD_NODE( CreateOpClassStmt, opfamilyname );
      TEXT_NODE( CreateOpClassStmt, amname );
      CHILD_NODE( CreateOpClassStmt, datatype );
      CHILD_NODE( CreateOpClassStmt, items );
      BOOL_NODE( CreateOpClassStmt, isDefault );
      break;
    case T_CreateOpFamilyStmt:
      CHILD_NODE( CreateOpFamilyStmt, opfamilyname );
      TEXT_NODE( CreateOpFamilyStmt, amname );
      break;
    case T_AlterOpFamilyStmt:
      CHILD_NODE( AlterOpFamilyStmt, opfamilyname );
      TEXT_NODE( AlterOpFamilyStmt, amname );
      BOOL_NODE( AlterOpFamilyStmt, isDrop );
      CHILD_NODE( AlterOpFamilyStmt, items );
      break;
    case T_RemoveOpClassStmt:
      CHILD_NODE( RemoveOpClassStmt, opclassname );
      TEXT_NODE( RemoveOpClassStmt, amname );
      xml_textnode( context, "behavior", "%s", DropBehavior_Names[((RemoveOpClassStmt*)node)->behavior] );
      BOOL_NODE( RemoveOpClassStmt, missing_ok );
      break;
    case T_RemoveOpFamilyStmt:
      CHILD_NODE( RemoveOpFamilyStmt, opfamilyname );
      TEXT_NODE( RemoveOpFamilyStmt, amname );
      xml_textnode( context, "behavior", "%s", DropBehavior_Names[((RemoveOpFamilyStmt*)node)->behavior] );
      BOOL_NODE( RemoveOpFamilyStmt, missing_ok );
      break;
    case T_PrepareStmt:
      TEXT_NODE( PrepareStmt, name );
      CHILD_NODE( PrepareStmt, argtypes );
      CHILD_NODE( PrepareStmt, query );
      break;
    case T_ExecuteStmt:
      TEXT_NODE( ExecuteStmt, name );
      CHILD_NODE( ExecuteStmt, into );
      CHILD_NODE( ExecuteStmt, params );
      break;
    case T_DeallocateStmt:
      TEXT_NODE( DeallocateStmt, name );
      break;
    case T_DeclareCursorStmt:
      TEXT_NODE( DeclareCursorStmt, portalname );
      NUMBER_NODE( DeclareCursorStmt, options );
      CHILD_NODE( DeclareCursorStmt, query );
      break;
    case T_CreateTableSpaceStmt:
      TEXT_NODE( CreateTableSpaceStmt, tablespacename );
      TEXT_NODE( CreateTableSpaceStmt, owner );
      TEXT_NODE( CreateTableSpaceStmt, location );
      break;
    case T_DropTableSpaceStmt:
      TEXT_NODE( DropTableSpaceStmt, tablespacename );
      BOOL_NODE( DropTableSpaceStmt, missing_ok );
      break;
    case T_AlterObjectSchemaStmt:
      xml_textnode( context, "objectType", "%s", ObjectType_Names[((AlterObjectSchemaStmt*)node)->objectType] );
      CHILD_NODE( AlterObjectSchemaStmt, relation );
      CHILD_NODE( AlterObjectSchemaStmt, object );
      CHILD_NODE( AlterObjectSchemaStmt, objarg );
      TEXT_NODE( AlterObjectSchemaStmt, addname );
      TEXT_NODE( AlterObjectSchemaStmt, newschema );
      break;
    case T_AlterOwnerStmt:
      xml_textnode( context, "objectType", "%s", ObjectType_Names[((AlterOwnerStmt*)node)->objectType] );
      CHILD_NODE( AlterOwnerStmt, relation );
      CHILD_NODE( AlterOwnerStmt, object );
      CHILD_NODE( AlterOwnerStmt, objarg );
      TEXT_NODE( AlterOwnerStmt, addname );
      TEXT_NODE( AlterOwnerStmt, newowner );
      break;
    case T_DropOwnedStmt:
      CHILD_NODE( DropOwnedStmt, roles );
      xml_textnode( context, "behavior", "%s", DropBehavior_Names[((DropOwnedStmt*)node)->behavior] );
      break;
    case T_ReassignOwnedStmt:
      CHILD_NODE( ReassignOwnedStmt, roles );
      TEXT_NODE( ReassignOwnedStmt, newrole );
      break;
    case T_CompositeTypeStmt:
      CHILD_NODE( CompositeTypeStmt, typevar );
      CHILD_NODE( CompositeTypeStmt, coldeflist );
      break;
    case T_CreateEnumStmt:
      CHILD_NODE( CreateEnumStmt, typename );
      CHILD_NODE( CreateEnumStmt, vals );
      break;
    case T_A_Expr:         // 900
      xml_textnode( context, "kind", "%s", A_Expr_Kind_Names[((A_Expr *)node)->kind] );
      CHILD_NODE( A_Expr, name );
      CHILD_NODE( A_Expr, lexpr );
      CHILD_NODE( A_Expr, rexpr );
      NUMBER_NODE( A_Expr, location );
      break;
    case T_ColumnRef:      // 901
      CHILD_NODE( ColumnRef, fields );
      NUMBER_NODE( ColumnRef, location );
      break;
    case T_ParamRef:       // 902
      NUMBER_NODE( ParamRef, number );
      NUMBER_NODE( ParamRef, location );
      break;
    case T_A_Const:        // 903
      parse_tree_walker((Node *) &(((A_Const *)node)->val), (void *) context );
      NUMBER_NODE( A_Const, location );
      break;
    case T_FuncCall:       // 904
      CHILD_NODE( FuncCall, funcname );
      CHILD_NODE( FuncCall, args );
      BOOL_NODE( FuncCall, agg_star );
      BOOL_NODE( FuncCall, agg_distinct );
      BOOL_NODE( FuncCall, func_variadic );
      CHILD_NODE( FuncCall, over );
      NUMBER_NODE( FuncCall, location );
      break;
    case T_A_Star:         // 905
      break;
    case T_A_Indices:      // 906
      CHILD_NODE( A_Indices, lidx );
      CHILD_NODE( A_Indices, uidx );
      break;
    case T_A_Indirection:  // 907
      CHILD_NODE( A_Indirection, arg );
      CHILD_NODE( A_Indirection, indirection );
      break;
    case T_A_ArrayExpr:   // 908
      CHILD_NODE( A_ArrayExpr, elements );
      NUMBER_NODE( A_ArrayExpr, location );
      break;
    case T_ResTarget:      // 909
      TEXT_NODE( ResTarget, name );
      CHILD_NODE( ResTarget, indirection );
      CHILD_NODE( ResTarget, val );
      NUMBER_NODE( ResTarget, location );
      break;
    case T_TypeCast:       // 910
      CHILD_NODE( TypeCast, arg );
      CHILD_NODE( TypeCast, typename );
      NUMBER_NODE( TypeCast, location );
      break;
    case T_SortBy:
      CHILD_NODE( SortBy, node );
      xml_textnode( context, "sortby_dir", "%s", SortByDir_Names[((SortBy *)node)->sortby_dir] );
      xml_textnode( context, "sortby_nulls", "%s", SortByNulls_Names[((SortBy *)node)->sortby_nulls] );
      CHILD_NODE( SortBy, useOp );
      NUMBER_NODE( SortBy, location );
      break;
    case T_RangeSubselect:
      CHILD_NODE( RangeSubselect, subquery );
      CHILD_NODE( RangeSubselect, alias );
      break;
    case T_RangeFunction:
      CHILD_NODE( RangeFunction, funccallnode );
      CHILD_NODE( RangeFunction, alias );
      CHILD_NODE( RangeFunction, coldeflist );
      break;
    case T_TypeName:
      CHILD_NODE( TypeName, names );
      NUMBER_NODE( TypeName, typeid );
      BOOL_NODE( TypeName, setof );
      BOOL_NODE( TypeName, pct_type );
      CHILD_NODE( TypeName, typmods );
      NUMBER_NODE( TypeName, typemod );
      CHILD_NODE( TypeName, arrayBounds );
      NUMBER_NODE( TypeName, location );
      break;
    case T_ColumnDef:
      TEXT_NODE( ColumnDef, colname );
      CHILD_NODE( ColumnDef, typename );
      NUMBER_NODE( ColumnDef, inhcount );
      BOOL_NODE( ColumnDef, is_local );
      BOOL_NODE( ColumnDef, is_not_null );
      CHILD_NODE( ColumnDef, raw_default );
      TEXT_NODE( ColumnDef, cooked_default );
      CHILD_NODE( ColumnDef, constraints );
      break;
    case T_IndexElem:
      TEXT_NODE( IndexElem, name );
      CHILD_NODE( IndexElem, expr );
      CHILD_NODE( IndexElem, opclass );
      xml_textnode( context, "ordering", "%s", SortByDir_Names[((IndexElem *)node)->ordering] );
      xml_textnode( context, "nulls_ordering", "%s", SortByNulls_Names[((IndexElem *)node)->nulls_ordering] );
      break;
    case T_Constraint:
      DICT_NODE( Constraint, contype, ConstrType_Names );
      TEXT_NODE( Constraint, name );
      CHILD_NODE( Constraint, raw_expr );
      TEXT_NODE( Constraint, cooked_expr );
      CHILD_NODE( Constraint, keys );
      CHILD_NODE( Constraint, options );
      TEXT_NODE( Constraint, indexspace );
      break;
    case T_DefElem:
      TEXT_NODE( DefElem, defnamespace );
      TEXT_NODE( DefElem, defname );
      CHILD_NODE( DefElem, arg );
      DICT_NODE( DefElem, defaction, DefElemAction_Names );
      break;
    case T_RangeTblEntry:
      DICT_NODE( RangeTblEntry, rtekind, RTEKind_Names );
      break;
    case T_SortGroupClause:
      NUMBER_NODE( SortGroupClause, tleSortGroupRef );
      NUMBER_NODE( SortGroupClause, eqop );
      NUMBER_NODE( SortGroupClause, sortop );
      BOOL_NODE( SortGroupClause, nulls_first );
      break;
    case T_FkConstraint:
      TEXT_NODE( FkConstraint, constr_name );
      CHILD_NODE( FkConstraint, pktable );
      CHILD_NODE( FkConstraint, fk_attrs );
      CHILD_NODE( FkConstraint, pk_attrs );
      CHAR_NODE( FkConstraint, fk_matchtype );
      CHAR_NODE( FkConstraint, fk_upd_action );
      CHAR_NODE( FkConstraint, fk_del_action );
      BOOL_NODE( FkConstraint, deferrable );
      BOOL_NODE( FkConstraint, initdeferred );
      BOOL_NODE( FkConstraint, skip_validation );
      break;
    case T_PrivGrantee:
      TEXT_NODE( PrivGrantee, rolname );
      break;
    case T_FuncWithArgs:
      CHILD_NODE( FuncWithArgs, funcname );
      CHILD_NODE( FuncWithArgs, funcargs );
      break;
    case T_AccessPriv:
      TEXT_NODE( AccessPriv, priv_name );
      CHILD_NODE( AccessPriv, cols );
      break;
    case T_CreateOpClassItem:
      NUMBER_NODE( CreateOpClassItem, itemtype );
      CHILD_NODE( CreateOpClassItem, name );
      CHILD_NODE( CreateOpClassItem, args );
      NUMBER_NODE( CreateOpClassItem, number );
      CHILD_NODE( CreateOpClassItem, class_args );
      CHILD_NODE( CreateOpClassItem, storedtype );
      break;
    case T_InhRelation:
      CHILD_NODE( InhRelation, relation );
      CHILD_NODE( InhRelation, options );
      break;
    case T_FunctionParameter:
      TEXT_NODE( FunctionParameter, name );
      CHILD_NODE( FunctionParameter, argType );
      DICT_NODE( FunctionParameter, mode , FunctionParameterMode_Names );
      CHILD_NODE( FunctionParameter, defexpr );
      break;
    case T_LockingClause:
      CHILD_NODE( LockingClause, lockedRels );
      BOOL_NODE( LockingClause, forUpdate );
      BOOL_NODE( LockingClause, noWait );
      break;
    case T_RowMarkClause:
      NUMBER_NODE( RowMarkClause, rti );
      NUMBER_NODE( RowMarkClause, prti );
      BOOL_NODE( RowMarkClause, forUpdate );
      BOOL_NODE( RowMarkClause, noWait );
      BOOL_NODE( RowMarkClause, isParent );
      break;
    case T_XmlSerialize:
      DICT_NODE( XmlSerialize, xmloption, XmlOptionType_Names );
      CHILD_NODE( XmlSerialize, expr );
      CHILD_NODE( XmlSerialize, typename );
      NUMBER_NODE( XmlSerialize, location );
      break;
    case T_WithClause:
      CHILD_NODE( WithClause, ctes );
      BOOL_NODE( WithClause, recursive );
      NUMBER_NODE( WithClause, location );
      break;
    case T_CommonTableExpr:
      TEXT_NODE( CommonTableExpr, ctename );
      CHILD_NODE( CommonTableExpr, aliascolnames );
      CHILD_NODE( CommonTableExpr, ctequery );
      NUMBER_NODE( CommonTableExpr, location );
      BOOL_NODE( CommonTableExpr, cterecursive );
      NUMBER_NODE( CommonTableExpr, cterefcount );
      CHILD_NODE( CommonTableExpr, ctecolnames );
      CHILD_NODE( CommonTableExpr, ctecoltypes );
      CHILD_NODE( CommonTableExpr, ctecoltypmods );
      break;

    default: 
      xml_tag( context, "unhandled_node", "id", "%d", nodeTag(node), "name", "%s", tagname, NULL );

  };
  xml_tag_close( context, tagname );

  return retval;
}

