
#include <postgres.h>
#include <nodes/nodes.h>
#include <nodes/parsenodes.h>


const char const * NodeTag_Names[] = {
  [T_Invalid] = "Invalid",
  [T_IndexInfo] = "IndexInfo",
  [T_ExprContext] = "ExprContext",
  [T_ProjectionInfo] = "ProjectionInfo",
  [T_JunkFilter] = "JunkFilter",
  [T_ResultRelInfo] = "ResultRelInfo",
  [T_EState] = "EState",
  [T_TupleTableSlot] = "TupleTableSlot",
  [T_Plan] = "Plan",
  [T_Result] = "Result",
  [T_Append] = "Append",
  [T_RecursiveUnion] = "RecursiveUnion",
  [T_BitmapAnd] = "BitmapAnd",
  [T_BitmapOr] = "BitmapOr",
  [T_Scan] = "Scan",
  [T_SeqScan] = "SeqScan",
  [T_IndexScan] = "IndexScan",
  [T_BitmapIndexScan] = "BitmapIndexScan",
  [T_BitmapHeapScan] = "BitmapHeapScan",
  [T_TidScan] = "TidScan",
  [T_SubqueryScan] = "SubqueryScan",
  [T_FunctionScan] = "FunctionScan",
  [T_ValuesScan] = "ValuesScan",
  [T_CteScan] = "CteScan",
  [T_WorkTableScan] = "WorkTableScan",
  [T_Join] = "Join",
  [T_NestLoop] = "NestLoop",
  [T_MergeJoin] = "MergeJoin",
  [T_HashJoin] = "HashJoin",
  [T_Material] = "Material",
  [T_Sort] = "Sort",
  [T_Group] = "Group",
  [T_Agg] = "Agg",
  [T_Unique] = "Unique",
  [T_Hash] = "Hash",
  [T_SetOp] = "SetOp",
  [T_Limit] = "Limit",
  [T_PlanInvalItem] = "PlanInvalItem",
  [T_PlanState] = "PlanState",
  [T_ResultState] = "ResultState",
  [T_AppendState] = "AppendState",
  [T_RecursiveUnionState] = "RecursiveUnionState",
  [T_BitmapAndState] = "BitmapAndState",
  [T_BitmapOrState] = "BitmapOrState",
  [T_ScanState] = "ScanState",
  [T_SeqScanState] = "SeqScanState",
  [T_IndexScanState] = "IndexScanState",
  [T_BitmapIndexScanState] = "BitmapIndexScanState",
  [T_BitmapHeapScanState] = "BitmapHeapScanState",
  [T_TidScanState] = "TidScanState",
  [T_SubqueryScanState] = "SubqueryScanState",
  [T_FunctionScanState] = "FunctionScanState",
  [T_ValuesScanState] = "ValuesScanState",
  [T_CteScanState] = "CteScanState",
  [T_WorkTableScanState] = "WorkTableScanState",
  [T_JoinState] = "JoinState",
  [T_NestLoopState] = "NestLoopState",
  [T_MergeJoinState] = "MergeJoinState",
  [T_HashJoinState] = "HashJoinState",
  [T_MaterialState] = "MaterialState",
  [T_SortState] = "SortState",
  [T_GroupState] = "GroupState",
  [T_AggState] = "AggState",
  [T_UniqueState] = "UniqueState",
  [T_HashState] = "HashState",
  [T_SetOpState] = "SetOpState",
  [T_LimitState] = "LimitState",
  [T_Alias] = "Alias",
  [T_RangeVar] = "RangeVar",
  [T_Expr] = "Expr",
  [T_Var] = "Var",
  [T_Const] = "Const",
  [T_Param] = "Param",
  [T_Aggref] = "Aggref",
  [T_ArrayRef] = "ArrayRef",
  [T_FuncExpr] = "FuncExpr",
  [T_OpExpr] = "OpExpr",
  [T_DistinctExpr] = "DistinctExpr",
  [T_ScalarArrayOpExpr] = "ScalarArrayOpExpr",
  [T_BoolExpr] = "BoolExpr",
  [T_SubLink] = "SubLink",
  [T_SubPlan] = "SubPlan",
  [T_AlternativeSubPlan] = "AlternativeSubPlan",
  [T_FieldSelect] = "FieldSelect",
  [T_FieldStore] = "FieldStore",
  [T_RelabelType] = "RelabelType",
  [T_CoerceViaIO] = "CoerceViaIO",
  [T_ArrayCoerceExpr] = "ArrayCoerceExpr",
  [T_ConvertRowtypeExpr] = "ConvertRowtypeExpr",
  [T_CaseExpr] = "CaseExpr",
  [T_CaseWhen] = "CaseWhen",
  [T_CaseTestExpr] = "CaseTestExpr",
  [T_ArrayExpr] = "ArrayExpr",
  [T_RowExpr] = "RowExpr",
  [T_RowCompareExpr] = "RowCompareExpr",
  [T_CoalesceExpr] = "CoalesceExpr",
  [T_MinMaxExpr] = "MinMaxExpr",
  [T_XmlExpr] = "XmlExpr",
  [T_NullIfExpr] = "NullIfExpr",
  [T_NullTest] = "NullTest",
  [T_BooleanTest] = "BooleanTest",
  [T_CoerceToDomain] = "CoerceToDomain",
  [T_CoerceToDomainValue] = "CoerceToDomainValue",
  [T_SetToDefault] = "SetToDefault",
  [T_CurrentOfExpr] = "CurrentOfExpr",
  [T_TargetEntry] = "TargetEntry",
  [T_RangeTblRef] = "RangeTblRef",
  [T_JoinExpr] = "JoinExpr",
  [T_FromExpr] = "FromExpr",
  [T_IntoClause] = "IntoClause",
  [T_ExprState] = "ExprState",
  [T_GenericExprState] = "GenericExprState",
  [T_AggrefExprState] = "AggrefExprState",
  [T_ArrayRefExprState] = "ArrayRefExprState",
  [T_FuncExprState] = "FuncExprState",
  [T_ScalarArrayOpExprState] = "ScalarArrayOpExprState",
  [T_BoolExprState] = "BoolExprState",
  [T_SubPlanState] = "SubPlanState",
  [T_AlternativeSubPlanState] = "AlternativeSubPlanState",
  [T_FieldSelectState] = "FieldSelectState",
  [T_FieldStoreState] = "FieldStoreState",
  [T_CoerceViaIOState] = "CoerceViaIOState",
  [T_ArrayCoerceExprState] = "ArrayCoerceExprState",
  [T_ConvertRowtypeExprState] = "ConvertRowtypeExprState",
  [T_CaseExprState] = "CaseExprState",
  [T_CaseWhenState] = "CaseWhenState",
  [T_ArrayExprState] = "ArrayExprState",
  [T_RowExprState] = "RowExprState",
  [T_RowCompareExprState] = "RowCompareExprState",
  [T_CoalesceExprState] = "CoalesceExprState",
  [T_MinMaxExprState] = "MinMaxExprState",
  [T_XmlExprState] = "XmlExprState",
  [T_NullTestState] = "NullTestState",
  [T_CoerceToDomainState] = "CoerceToDomainState",
  [T_DomainConstraintState] = "DomainConstraintState",
  [T_PlannerInfo] = "PlannerInfo", // 500
  [T_PlannerGlobal] = "PlannerGlobal",
  [T_RelOptInfo] = "RelOptInfo",
  [T_IndexOptInfo] = "IndexOptInfo",
  [T_Path] = "Path",
  [T_IndexPath] = "IndexPath",
  [T_BitmapHeapPath] = "BitmapHeapPath",
  [T_BitmapAndPath] = "BitmapAndPath",
  [T_BitmapOrPath] = "BitmapOrPath",
  [T_NestPath] = "NestPath",
  [T_MergePath] = "MergePath",
  [T_HashPath] = "HashPath",
  [T_TidPath] = "TidPath",
  [T_AppendPath] = "AppendPath",
  [T_ResultPath] = "ResultPath",
  [T_MaterialPath] = "MaterialPath",
  [T_UniquePath] = "UniquePath",
  [T_EquivalenceClass] = "EquivalenceClass",
  [T_EquivalenceMember] = "EquivalenceMember",
  [T_PathKey] = "PathKey",
  [T_RestrictInfo] = "RestrictInfo",
  [T_InnerIndexscanInfo] = "InnerIndexscanInfo",
  [T_PlaceHolderVar] = "PlaceHolderVar",
  [T_SpecialJoinInfo] = "SpecialJoinInfo",
  [T_AppendRelInfo] = "AppendRelInfo",
  [T_PlaceHolderInfo] = "PlaceHolderInfo",
  [T_PlannerParamItem] = "PlannerParamItem",
  [T_MemoryContext] = "MemoryContext",  // 600
  [T_AllocSetContext] = "AllocSetContext",
  [T_Value] = "Value", // 650
  [T_Integer] = "Integer",
  [T_Float] = "Float",
  [T_String] = "String",
  [T_BitString] = "BitString",
  [T_Null] = "Null",
  [T_List] = "List",
  [T_IntList] = "IntList",
  [T_OidList] = "OidList",
  [T_Query] = "Query", // 700
  [T_PlannedStmt] = "PlannedStmt",
  [T_InsertStmt] = "InsertStmt",
  [T_DeleteStmt] = "DeleteStmt",
  [T_UpdateStmt] = "UpdateStmt",
  [T_SelectStmt] = "SelectStmt",
  [T_AlterTableStmt] = "AlterTableStmt",
  [T_AlterTableCmd] = "AlterTableCmd",
  [T_AlterDomainStmt] = "AlterDomainStmt",
  [T_SetOperationStmt] = "SetOperationStmt",
  [T_GrantStmt] = "GrantStmt",
  [T_GrantRoleStmt] = "GrantRoleStmt",
  [T_ClosePortalStmt] = "ClosePortalStmt",
  [T_ClusterStmt] = "ClusterStmt",
  [T_CopyStmt] = "CopyStmt",
  [T_CreateStmt] = "CreateStmt",
  [T_DefineStmt] = "DefineStmt",
  [T_DropStmt] = "DropStmt",
  [T_TruncateStmt] = "TruncateStmt",
  [T_CommentStmt] = "CommentStmt",
  [T_FetchStmt] = "FetchStmt",
  [T_IndexStmt] = "IndexStmt",
  [T_CreateFunctionStmt] = "CreateFunctionStmt",
  [T_AlterFunctionStmt] = "AlterFunctionStmt",
  [T_RemoveFuncStmt] = "RemoveFuncStmt",
  [T_RenameStmt] = "RenameStmt",
  [T_RuleStmt] = "RuleStmt",
  [T_NotifyStmt] = "NotifyStmt",
  [T_ListenStmt] = "ListenStmt",
  [T_UnlistenStmt] = "UnlistenStmt",
  [T_TransactionStmt] = "TransactionStmt",
  [T_ViewStmt] = "ViewStmt",
  [T_LoadStmt] = "LoadStmt",
  [T_CreateDomainStmt] = "CreateDomainStmt",
  [T_CreatedbStmt] = "CreatedbStmt",
  [T_DropdbStmt] = "DropdbStmt",
  [T_VacuumStmt] = "VacuumStmt",
  [T_ExplainStmt] = "ExplainStmt",
  [T_CreateSeqStmt] = "CreateSeqStmt",
  [T_AlterSeqStmt] = "AlterSeqStmt",
  [T_VariableSetStmt] = "VariableSetStmt",
  [T_VariableShowStmt] = "VariableShowStmt",
  [T_DiscardStmt] = "DiscardStmt",
  [T_CreateTrigStmt] = "CreateTrigStmt",
  [T_DropPropertyStmt] = "DropPropertyStmt",
  [T_CreatePLangStmt] = "CreatePLangStmt",
  [T_DropPLangStmt] = "DropPLangStmt",
  [T_CreateRoleStmt] = "CreateRoleStmt",
  [T_AlterRoleStmt] = "AlterRoleStmt",
  [T_DropRoleStmt] = "DropRoleStmt",
  [T_LockStmt] = "LockStmt",
  [T_ConstraintsSetStmt] = "ConstraintsSetStmt",
  [T_ReindexStmt] = "ReindexStmt",
  [T_CheckPointStmt] = "CheckPointStmt",
  [T_CreateSchemaStmt] = "CreateSchemaStmt",
  [T_AlterDatabaseStmt] = "AlterDatabaseStmt",
  [T_AlterDatabaseSetStmt] = "AlterDatabaseSetStmt",
  [T_AlterRoleSetStmt] = "AlterRoleSetStmt",
  [T_CreateConversionStmt] = "CreateConversionStmt",
  [T_CreateCastStmt] = "CreateCastStmt",
  [T_DropCastStmt] = "DropCastStmt",
  [T_CreateOpClassStmt] = "CreateOpClassStmt",
  [T_CreateOpFamilyStmt] = "CreateOpFamilyStmt",
  [T_AlterOpFamilyStmt] = "AlterOpFamilyStmt",
  [T_RemoveOpClassStmt] = "RemoveOpClassStmt",
  [T_RemoveOpFamilyStmt] = "RemoveOpFamilyStmt",
  [T_PrepareStmt] = "PrepareStmt",
  [T_ExecuteStmt] = "ExecuteStmt",
  [T_DeallocateStmt] = "DeallocateStmt",
  [T_DeclareCursorStmt] = "DeclareCursorStmt",
  [T_CreateTableSpaceStmt] = "CreateTableSpaceStmt",
  [T_DropTableSpaceStmt] = "DropTableSpaceStmt",
  [T_AlterObjectSchemaStmt] = "AlterObjectSchemaStmt",
  [T_AlterOwnerStmt] = "AlterOwnerStmt",
  [T_DropOwnedStmt] = "DropOwnedStmt",
  [T_ReassignOwnedStmt] = "ReassignOwnedStmt",
  [T_CompositeTypeStmt] = "CompositeTypeStmt",
  [T_CreateEnumStmt] = "CreateEnumStmt",
  [T_AlterTSDictionaryStmt] = "AlterTSDictionaryStmt",
  [T_AlterTSConfigurationStmt] = "AlterTSConfigurationStmt",
  [T_A_Expr] = "A_Expr",
  [T_ColumnRef] = "ColumnRef",
  [T_ParamRef] = "ParamRef",
  [T_A_Const] = "A_Const",
  [T_FuncCall] = "FuncCall",
  [T_A_Star] = "A_Star",
  [T_A_Indices] = "A_Indices",
  [T_A_Indirection] = "A_Indirection",
  [T_A_ArrayExpr] = "A_ArrayExpr",
  [T_ResTarget] = "ResTarget",
  [T_TypeCast] = "TypeCast",
  [T_SortBy] = "SortBy",
  [T_RangeSubselect] = "RangeSubselect",
  [T_RangeFunction] = "RangeFunction",
  [T_TypeName] = "TypeName",
  [T_ColumnDef] = "ColumnDef",
  [T_IndexElem] = "IndexElem",
  [T_Constraint] = "Constraint",
  [T_DefElem] = "DefElem",
  [T_RangeTblEntry] = "RangeTblEntry",
  [T_SortGroupClause] = "SortGroupClause",
  [T_FkConstraint] = "FkConstraint",
  [T_PrivGrantee] = "PrivGrantee",
  [T_FuncWithArgs] = "FuncWithArgs",
  [T_AccessPriv] = "AccessPriv",
  [T_CreateOpClassItem] = "CreateOpClassItem",
  [T_InhRelation] = "InhRelation",
  [T_FunctionParameter] = "FunctionParameter",
  [T_LockingClause] = "LockingClause",
  [T_RowMarkClause] = "RowMarkClause",
  [T_XmlSerialize] = "XmlSerialize",
  [T_WithClause] = "WithClause",
  [T_CommonTableExpr] = "CommonTableExpr",
  [T_TriggerData] = "TriggerData",
  [T_ReturnSetInfo] = "ReturnSetInfo",
  [T_TIDBitmap] = "TIDBitmap"
};

const char * ParamKind_Names[] = {
  [PARAM_EXTERN] = "PARAM_EXTERN",
  [PARAM_EXEC] = "PARAM_EXEC",
  [PARAM_SUBLINK] = "PARAM_SUBLINK"  
};

const char * CoercionForm_Names[] = {
  [COERCE_EXPLICIT_CALL] = "COERCE_EXPLICIT_CAST",
  [COERCE_EXPLICIT_CAST] = "COERCE_EXPLICIT_CALL",
  [COERCE_IMPLICIT_CAST] = "COERCE_IMPLICIT_CAST",
  [COERCE_DONTCARE] = "COERCE_DONTCARE"
};

const char * BoolExprType_Names[] = {
  [AND_EXPR] = "AND_EXPR",
  [OR_EXPR] = "OR_EXPR",
  [NOT_EXPR] = "NOT_EXPR"  
};

const char * SubLinkType_Names[] = {
  [EXISTS_SUBLINK] = "EXISTS_SUBLINK",
  [ALL_SUBLINK] = "ALL_SUBLINK",
  [ANY_SUBLINK] = "ANY_SUBLINK",
  [ROWCOMPARE_SUBLINK] = "ROWCOMPARE_SUBLINK",
  [EXPR_SUBLINK] = "EXPR_SUBLINK",
  [ARRAY_SUBLINK] = "ARRAY_SUBLINK",
  [CTE_SUBLINK] = "CTE_SUBLINK"  
};

const char * RowCompareType_Names[] = {
  [ROWCOMPARE_LT] = "ROWCOMPARE_LT",  
  [ROWCOMPARE_LE] = "ROWCOMPARE_LE",  
  [ROWCOMPARE_EQ] = "ROWCOMPARE_EQ",  
  [ROWCOMPARE_GE] = "ROWCOMPARE_GE",  
  [ROWCOMPARE_GT] = "ROWCOMPARE_GT",  
  [ROWCOMPARE_NE] = "ROWCOMPARE_NE"
};

const char * MinMaxOp_Names[] = {
  [IS_GREATEST] = "IS_GREATEST",
  [IS_LEAST] = "IS_LEAST"  
};

const char * XmlExprOp_Names[] = {
  [IS_XMLCONCAT] = "IS_XMLCONCAT",
  [IS_XMLELEMENT] = "IS_XMLELEMENT",  
  [IS_XMLFOREST] = "IS_XMLFOREST",
  [IS_XMLPARSE] = "IS_XMLPARSE",
  [IS_XMLPI] = "IS_XMLPI",
  [IS_XMLROOT] = "IS_XMLROOT",
  [IS_XMLSERIALIZE] = "IS_XMLSERIALIZE",
  [IS_DOCUMENT] = "IS_DOCUMENT"
};

const char * XmlOptionType_Names[] = {
  [XMLOPTION_DOCUMENT] = "XMLOPTION_DOCUMENT",
  [XMLOPTION_CONTENT] = "XMLOPTION_CONTENT"  
};

const char * NullTestType_Names[] = {
  [IS_NULL] = "IS_NULL",
  [IS_NOT_NULL] = "IS_NOT_NULL"  
};

const char * BoolTestType_Names[] = {
  [IS_TRUE] = "IS_TRUE",
  [IS_NOT_TRUE] = "IS_NOT_TRUE",
  [IS_FALSE] = "IS_FALSE",
  [IS_NOT_FALSE] = "IS_NOT_FALSE",
  [IS_UNKNOWN] = "IS_UNKNOWN",
  [IS_NOT_UNKNOWN] = "IS_NOT_UNKNOWN"
};

const char * OnCommitAction_Names[] = {
  [ONCOMMIT_NOOP] = "ONCOMMIT_NOOP",
  [ONCOMMIT_PRESERVE_ROWS] = "ONCOMMIT_PRESERVE_ROWS",
  [ONCOMMIT_DELETE_ROWS] = "ONCOMMIT_DELETE_ROWS",
  [ONCOMMIT_DROP] = "ONCOMMIT_DROP"  
};

const char * CmdType_Names[] = {
  [CMD_UNKNOWN] = "CMD_UNKNOWN",
  [CMD_SELECT] = "CMD_SELECT",
  [CMD_UPDATE] = "CMD_UPDATE",
  [CMD_INSERT] = "CMD_INSERT",
  [CMD_DELETE] = "CMD_DELETE",
  [CMD_UTILITY] = "CMD_UTILITY",
  [CMD_NOTHING] = "CMD_NOTHING"
};

const char * QuerySource_Names[] = {
  [QSRC_ORIGINAL] = "QSRC_ORIGINAL",
  [QSRC_PARSER] = "QSRC_PARSER",
  [QSRC_INSTEAD_RULE] = "QSRC_INSTEAD_RULE",
  [QSRC_QUAL_INSTEAD_RULE] = "QSRC_QUAL_INSTEAD_RULE",
  [QSRC_NON_INSTEAD_RULE] = "QSRC_NON_INSTEAD_RULE"  
};

const char * ObjectType_Names[] = {
  [OBJECT_AGGREGATE] = "OBJECT_AGGREGATE",
  [OBJECT_CAST] = "OBJECT_CAST",
  [OBJECT_COLUMN] = "OBJECT_COLUMN",
  [OBJECT_CONSTRAINT] = "OBJECT_CONSTRAINT",
  [OBJECT_CONVERSION] = "OBJECT_CONVERSION",
  [OBJECT_DATABASE] = "OBJECT_DATABASE",
  [OBJECT_DOMAIN] = "OBJECT_DOMAIN",
  [OBJECT_FDW] = "OBJECT_FDW",
  [OBJECT_FOREIGN_SERVER] = "OBJECT_FOREIGN_SERVER",
  [OBJECT_FUNCTION] = "OBJECT_FUNCTION",
  [OBJECT_INDEX] = "OBJECT_INDEX",
  [OBJECT_LANGUAGE] = "OBJECT_LANGUAGE",
  [OBJECT_LARGEOBJECT] = "OBJECT_LARGEOBJECT",
  [OBJECT_OPCLASS] = "OBJECT_OPCLASS",
  [OBJECT_OPERATOR] = "OBJECT_OPERATOR",
  [OBJECT_OPFAMILY] = "OBJECT_OPFAMILY",
  [OBJECT_ROLE] = "OBJECT_ROLE",
  [OBJECT_RULE] = "OBJECT_RULE",
  [OBJECT_SCHEMA] = "OBJECT_SCHEMA",
  [OBJECT_SEQUENCE] = "OBJECT_SEQUENCE",
  [OBJECT_TABLE] = "OBJECT_TABLE",
  [OBJECT_TABLESPACE] = "OBJECT_TABLESPACE",
  [OBJECT_TRIGGER] = "OBJECT_TRIGGER",
  [OBJECT_TSCONFIGURATION] = "OBJECT_TSCONFIGURATION",
  [OBJECT_TSDICTIONARY] = "OBJECT_TSDICTIONARY",
  [OBJECT_TSPARSER] = "OBJECT_TSPARSER",
  [OBJECT_TSTEMPLATE] = "OBJECT_TSTEMPLATE",
  [OBJECT_TYPE] = "OBJECT_TYPE",
  [OBJECT_VIEW] = "OBJECT_VIEW"
};

const char * SetOperation_Names[] = {
  [SETOP_NONE] = "NONE",
  [SETOP_UNION] = "UNION",
  [SETOP_INTERSECT] = "INTERSECT",
  [SETOP_EXCEPT] = "EXCEPT"
};

const char * AlterTableType_Names[] = {
  [AT_AddColumn] = "AT_AddColumn",       /* add column */
  [AT_AddColumnToView] = "AT_AddColumnToView",     /* implicitly via CREATE OR REPLACE VIEW */
  [AT_ColumnDefault] = "AT_ColumnDefault",     /* alter column default */
  [AT_DropNotNull] = "AT_DropNotNull",       /* alter column drop not null */
  [AT_SetNotNull] = "AT_SetNotNull",        /* alter column set not null */
  [AT_SetStatistics] = "AT_SetStatistics",     /* alter column statistics */
  [AT_SetStorage] = "AT_SetStorage",        /* alter column storage */
  [AT_DropColumn] = "AT_DropColumn",        /* drop column */
  [AT_DropColumnRecurse] = "AT_DropColumnRecurse",   /* internal to commands/tablecmds.c */
  [AT_AddIndex] = "AT_AddIndex",        /* add index */
  [AT_ReAddIndex] = "AT_ReAddIndex",        /* internal to commands/tablecmds.c */
  [AT_AddConstraint] = "AT_AddConstraint",     /* add constraint */
  [AT_AddConstraintRecurse] = "AT_AddConstraintRecurse",  /* internal to commands/tablecmds.c */
  [AT_ProcessedConstraint] = "AT_ProcessedConstraint",   /* pre-processed add constraint (local in
                   * parser/parse_utilcmd.c) */
  [AT_DropConstraint] = "AT_DropConstraint",      /* drop constraint */
  [AT_DropConstraintRecurse] = "AT_DropConstraintRecurse", /* internal to commands/tablecmds.c */
  [AT_AlterColumnType] = "AT_AlterColumnType",     /* alter column type */
  [AT_ChangeOwner] = "AT_ChangeOwner",       /* change owner */
  [AT_ClusterOn] = "AT_ClusterOn",       /* CLUSTER ON */
  [AT_DropCluster] = "AT_DropCluster",       /* SET WITHOUT CLUSTER */
  [AT_AddOids] = "AT_AddOids",         /* SET WITH OIDS */
  [AT_DropOids] = "AT_DropOids",        /* SET WITHOUT OIDS */
  [AT_SetTableSpace] = "AT_SetTableSpace",     /* SET TABLESPACE */
  [AT_SetRelOptions] = "AT_SetRelOptions",     /* SET (...) -- AM specific parameters */
  [AT_ResetRelOptions] = "AT_ResetRelOptions",     /* RESET (...) -- AM specific parameters */
  [AT_EnableTrig] = "AT_EnableTrig",        /* ENABLE TRIGGER name */
  [AT_EnableAlwaysTrig] = "AT_EnableAlwaysTrig",    /* ENABLE ALWAYS TRIGGER name */
  [AT_EnableReplicaTrig] = "AT_EnableReplicaTrig",   /* ENABLE REPLICA TRIGGER name */
  [AT_DisableTrig] = "AT_DisableTrig",       /* DISABLE TRIGGER name */
  [AT_EnableTrigAll] = "AT_EnableTrigAll",     /* ENABLE TRIGGER ALL */
  [AT_DisableTrigAll] = "AT_DisableTrigAll",      /* DISABLE TRIGGER ALL */
  [AT_EnableTrigUser] = "AT_EnableTrigUser",      /* ENABLE TRIGGER USER */
  [AT_DisableTrigUser] = "AT_DisableTrigUser",     /* DISABLE TRIGGER USER */
  [AT_EnableRule] = "AT_EnableRule",        /* ENABLE RULE name */
  [AT_EnableAlwaysRule] = "AT_EnableAlwaysRule",    /* ENABLE ALWAYS RULE name */
  [AT_EnableReplicaRule] = "AT_EnableReplicaRule",   /* ENABLE REPLICA RULE name */
  [AT_DisableRule] = "AT_DisableRule",       /* DISABLE RULE name */
  [AT_AddInherit] = "AT_AddInherit",        /* INHERIT parent */
  [AT_DropInherit] = "AT_DropInherit"        /* NO INHERIT parent */
};

const char * DropBehavior_Names[] = {
  [DROP_RESTRICT] = "DROP_RESTRICT",
  [DROP_CASCADE] = "DROP_CASCADE"
};

const char * GrantObjectType_Names[] = {
  [ACL_OBJECT_COLUMN] = "ACL_OBJECT_COLUMN",      /* column */
  [ACL_OBJECT_RELATION] = "ACL_OBJECT_RELATION",    /* table, view */
  [ACL_OBJECT_SEQUENCE] = "ACL_OBJECT_SEQUENCE",    /* sequence */
  [ACL_OBJECT_DATABASE] = "ACL_OBJECT_DATABASE",    /* database */
  [ACL_OBJECT_FDW] = "ACL_OBJECT_FDW",       /* foreign-data wrapper */
  [ACL_OBJECT_FOREIGN_SERVER] = "ACL_OBJECT_FOREIGN_SERVER",  /* foreign server */
  [ACL_OBJECT_FUNCTION] = "ACL_OBJECT_FUNCTION",    /* function */
  [ACL_OBJECT_LANGUAGE] = "ACL_OBJECT_LANGUAGE",    /* procedural language */
  [ACL_OBJECT_NAMESPACE] = "ACL_OBJECT_NAMESPACE",   /* namespace */
  [ACL_OBJECT_TABLESPACE] = "ACL_OBJECT_TABLESPACE"   /* tablespace */
};

const char * FetchDirection_Names[] = {
  [FETCH_FORWARD] = "FORWARD",
  [FETCH_BACKWARD] = "BACKWARD",
  [FETCH_ABSOLUTE] = "ABSOLUTE",
  [FETCH_RELATIVE] = "RELATIVE"
};

const char * TransactionStmtKind_Names[] = {
  [TRANS_STMT_BEGIN] = "TRANS_STMT_BEGIN",
  [TRANS_STMT_START] = "TRANS_STMT_START",     /* semantically identical to BEGIN */
  [TRANS_STMT_COMMIT] = "TRANS_STMT_COMMIT",
  [TRANS_STMT_ROLLBACK] = "TRANS_STMT_ROLLBACK",
  [TRANS_STMT_SAVEPOINT] = "TRANS_STMT_SAVEPOINT",
  [TRANS_STMT_RELEASE] = "TRANS_STMT_RELEASE",
  [TRANS_STMT_ROLLBACK_TO] = "TRANS_STMT_ROLLBACK_TO",
  [TRANS_STMT_PREPARE] = "TRANS_STMT_PREPARE",
  [TRANS_STMT_COMMIT_PREPARED] = "TRANS_STMT_COMMIT_PREPARED",
  [TRANS_STMT_ROLLBACK_PREPARED] = "TRANS_STMT_ROLLBACK_PREPARED"
};

const char * VariableSetKind_Names[] = {
  [VAR_SET_VALUE] = "VAR_SET_VALUE",        /* SET var = value */
  [VAR_SET_DEFAULT] = "VAR_SET_DEFAULT",      /* SET var TO DEFAULT */
  [VAR_SET_CURRENT] = "VAR_SET_CURRENT",      /* SET var FROM CURRENT */
  [VAR_SET_MULTI] = "VAR_SET_MULTI",        /* special case for SET TRANSACTION ... */
  [VAR_RESET] = "VAR_RESET",          /* RESET var */
  [VAR_RESET_ALL] = "VAR_RESET_ALL"       /* RESET ALL */
};

const char * DiscardMode_Names[] = {
  [DISCARD_ALL] = "DISCARD_ALL",
  [DISCARD_PLANS] = "DISCARD_PLANS",
  [DISCARD_TEMP] = "DISCARD_TEMP"
};

const char * RoleStmtType_Names[] = {
  [ROLESTMT_ROLE] = "ROLESTMT_ROLE",
  [ROLESTMT_USER] = "ROLESTMT_USER",
  [ROLESTMT_GROUP] = "ROLESTMT_GROUP"
};

const char * CoercionContext_Names[] = {
  [COERCION_IMPLICIT] = "COERCION_IMPLICIT",      /* coercion in context of expression */
  [COERCION_ASSIGNMENT] = "COERCION_ASSIGNMENT",    /* coercion in context of assignment */
  [COERCION_EXPLICIT] = "COERCION_EXPLICIT"     /* explicit cast operation */
};

const char * A_Expr_Kind_Names[] = {
  [AEXPR_OP] = "EXPR_OP",
  [AEXPR_AND] = "EXPR_AND",
  [AEXPR_OR] = "EXPR_OR",
  [AEXPR_NOT] = "EXPR_NOT",
  [AEXPR_OP_ANY] = "EXPR_OP_ANY",
  [AEXPR_OP_ALL] = "EXPR_OP_ALL",
  [AEXPR_DISTINCT] = "EXPR_DISTINCT",
  [AEXPR_NULLIF] = "EXPR_NULLIF",
  [AEXPR_OF] = "EXPR_OF",
  [AEXPR_IN] = "EXPR_IN"
};

const char * SortByDir_Names[] = {
  [SORTBY_DEFAULT] = "SORTBY_DEFAULT",
  [SORTBY_ASC] = "SORTBY_ASC",
  [SORTBY_DESC] = "SORTBY_DESC",
  [SORTBY_USING] = "SORTBY_USING"        /* not allowed in CREATE INDEX ... */
};

const char * SortByNulls_Names[] = {
  [SORTBY_NULLS_DEFAULT] = "SORTBY_NULLS_DEFAULT",
  [SORTBY_NULLS_FIRST] = "SORTBY_NULLS_FIRST",
  [SORTBY_NULLS_LAST] = "SORTBY_NULLS_LAST"
};

const char * JoinType_Names[] = {
  [JOIN_INNER] = "INNER",
  [JOIN_LEFT]  = "LEFT",
  [JOIN_FULL]  = "FULL",
  [JOIN_RIGHT] = "RIGHT",
  [JOIN_SEMI]  = "SEMI",
  [JOIN_ANTI]  = "ANTI",
  [JOIN_UNIQUE_OUTER] = "UNIQUE_OUTER",
  [JOIN_UNIQUE_INNER] = "UNIQUE_INNER"
};

const char * ConstrType_Names[] = {
  [CONSTR_NULL] = "CONSTR_NULL",        /* not SQL92, but a lot of people expect it */
  [CONSTR_NOTNULL] = "CONSTR_NOTNULL",
  [CONSTR_DEFAULT] = "CONSTR_DEFAULT",
  [CONSTR_CHECK] = "CONSTR_CHECK",
  [CONSTR_FOREIGN] = "CONSTR_FOREIGN",
  [CONSTR_PRIMARY] = "CONSTR_PRIMARY",
  [CONSTR_UNIQUE] = "CONSTR_UNIQUE",
  [CONSTR_ATTR_DEFERRABLE] = "CONSTR_ATTR_DEFERRABLE",   /* attributes for previous constraint node */
  [CONSTR_ATTR_NOT_DEFERRABLE] = "CONSTR_ATTR_NOT_DEFERRABLE",
  [CONSTR_ATTR_DEFERRED] = "CONSTR_ATTR_DEFERRED",
  [CONSTR_ATTR_IMMEDIATE] = "CONSTR_ATTR_IMMEDIATE"
};

const char * DefElemAction_Names[] = {
  [DEFELEM_UNSPEC] = "DEFELEM_UNSPEC",       /* no action given */
  [DEFELEM_SET] = "DEFELEM_SET",
  [DEFELEM_ADD] = "DEFELEM_ADD",
  [DEFELEM_DROP] = "DEFELEM_DROP"
};

const char * RTEKind_Names[] = {
  [RTE_RELATION] = "RTE_RELATION",       /* ordinary relation reference */
  [RTE_SUBQUERY] = "RTE_SUBQUERY",       /* subquery in FROM */
  [RTE_JOIN] = "RTE_JOIN",         /* join */
  [RTE_SPECIAL] = "RTE_SPECIAL",        /* special rule relation (NEW or OLD) */
  [RTE_FUNCTION] = "RTE_FUNCTION",       /* function in FROM */
  [RTE_VALUES] = "RTE_VALUES",         /* VALUES (<exprlist>), (<exprlist>), ... */
  [RTE_CTE] = "RTE_CTE"           /* common table expr (WITH list element) */
};

const char * FunctionParameterMode_Names[] = {
  [FUNC_PARAM_IN] = "FUNC_PARAM_IN",    /* input only */
  [FUNC_PARAM_OUT] = "FUNC_PARAM_OUT",   /* output only */
  [FUNC_PARAM_INOUT] = "FUNC_PARAM_INOUT",   /* both */
  [FUNC_PARAM_VARIADIC] = "FUNC_PARAM_VARIADIC",  /* variadic (always input) */
  [FUNC_PARAM_TABLE] = "FUNC_PARAM_TABLE"    /* table function output column */
};

