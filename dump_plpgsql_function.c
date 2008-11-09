
#include <postgres.h>
#include <fmgr.h>
#include <parser/parse_type.h>
#include <nodes/nodeFuncs.h>
#include <plpgsql.h>
#include <lib/stringinfo.h>

#include "dump_plpgsql_function.h"
#include "dump_sql_parse_tree.h"
#include "string_helper.h"

#define ROOTNODENAME "function_tree"



PG_MODULE_MAGIC;


/*
 * CREATE OR REPLACE FUNCTION dump_plpgsql_function(text) returns text AS '/home/sven/diplom/dump_plpgsql_function/dump_plpgsql_function.so', 'dump_plpgsql_function' LANGUAGE C STRICT;
 *
 */

typedef struct dump_context {
  PLpgSQL_function * func;
  char ** output;
} FunctionDumpContext;

static char * dumptree(PLpgSQL_function *func);

static void dump_stmt( FunctionDumpContext dump, PLpgSQL_stmt *stmt);
static void dump_block( FunctionDumpContext dump, PLpgSQL_stmt_block *block);
static void dump_loop( FunctionDumpContext dump, PLpgSQL_stmt_loop *stmt);
static void dump_while( FunctionDumpContext dump, PLpgSQL_stmt_while *stmt);
static void dump_fori( FunctionDumpContext dump, PLpgSQL_stmt_fori *stmt);
static void dump_fors( FunctionDumpContext dump, PLpgSQL_stmt_fors *stmt);
static void dump_exit( FunctionDumpContext dump, PLpgSQL_stmt_exit *stmt);
static void dump_return( FunctionDumpContext dump, PLpgSQL_stmt_return *stmt);
static void dump_return_next( FunctionDumpContext dump, PLpgSQL_stmt_return_next *stmt);
static void dump_return_query( FunctionDumpContext dump, PLpgSQL_stmt_return_query *stmt);
static void dump_raise( FunctionDumpContext dump, PLpgSQL_stmt_raise *stmt);
static void dump_execsql( FunctionDumpContext dump, PLpgSQL_stmt_execsql *stmt);
static void dump_dynexecute( FunctionDumpContext dump, PLpgSQL_stmt_dynexecute *stmt);
static void dump_dynfors( FunctionDumpContext dump, PLpgSQL_stmt_dynfors *stmt);
static void dump_getdiag( FunctionDumpContext dump, PLpgSQL_stmt_getdiag *stmt);
static void dump_open( FunctionDumpContext dump, PLpgSQL_stmt_open *stmt);
static void dump_fetch( FunctionDumpContext dump, PLpgSQL_stmt_fetch *stmt);
static void dump_cursor_direction( FunctionDumpContext dump, PLpgSQL_stmt_fetch *stmt);
static void dump_close( FunctionDumpContext dump, PLpgSQL_stmt_close *stmt);
static void dump_perform( FunctionDumpContext dump, PLpgSQL_stmt_perform *stmt);
static void dump_expr( FunctionDumpContext dump, PLpgSQL_expr *expr );
static void dump_assign( FunctionDumpContext dump, PLpgSQL_stmt_assign *stmt);
static void dump_if( FunctionDumpContext dump, PLpgSQL_stmt_if *stmt);


PG_FUNCTION_INFO_V1(dump_plpgsql_function);


Datum dump_plpgsql_function( PG_FUNCTION_ARGS )
{
  unsigned int funcoid = PG_GETARG_OID( 0 );

  FunctionCallInfoData fake_fcinfo;
  FmgrInfo    flinfo;
 
  /*
   * Set up a fake fcinfo with just enough info to satisfy
   * plpgsql_compile().
   */
  MemSet(&fake_fcinfo, 0, sizeof(fake_fcinfo));
  MemSet(&flinfo, 0, sizeof(flinfo));
  fake_fcinfo.flinfo = &flinfo;
  flinfo.fn_oid = funcoid;
  flinfo.fn_mcxt = CurrentMemoryContext;
 
  PLpgSQL_function * func;
  func = plpgsql_compile(&fake_fcinfo, true);
 
  char * result = dumptree( func );

  PG_RETURN_TEXT_P( cstring_to_text( result ) );
}


static char * dumptree(PLpgSQL_function *func)
{
  FunctionDumpContext dump;
  char * output = NULL;
  dump.func = func;
  dump.output = &output;
  int i;
  PLpgSQL_datum *d;

  append_string( dump.output, "<%s function_name=\"%s\">", ROOTNODENAME, func->fn_name ); 

  append_string( dump.output, "<data_area>" ); 
  for (i = 0; i < func->ndatums; i++)
  {
    d = func->datums[i];

    append_string( dump.output, "<entry index=\"%d\">", i );
    switch (d->dtype)
    {
      case PLPGSQL_DTYPE_VAR:
        {
          PLpgSQL_var *var = (PLpgSQL_var *) d;

          append_string( dump.output, "<variable name=\"%s\" datatype=\"%s\" datatype_oid=\"%u\" atttypmod=\"%d\" constant=\"%s\" not_null=\"%s\">",
              var->refname, var->datatype->typname, 
              var->datatype->typoid, var->datatype->atttypmod,
              var->isconst ? "yes" : "no", var->notnull ? "yes" : "no" );
          if (var->default_val != NULL)
          {
            append_string( dump.output, "<default>" ); 
            dump_expr( dump, var->default_val);
            append_string( dump.output, "</default>" ); 
          }
          if (var->cursor_explicit_expr != NULL)
          {
            if (var->cursor_explicit_argrow >= 0)
              printf("                                  CURSOR argument row %d\n", var->cursor_explicit_argrow);

            printf("                                  CURSOR IS ");
            dump_expr( dump, var->cursor_explicit_expr);
            printf("\n");
          }
        }
        append_string( dump.output, "</variable>" ); 
        break;
      case PLPGSQL_DTYPE_ROW:
        {
          PLpgSQL_row *row = (PLpgSQL_row *) d;
          int      i;

          printf("ROW %-16s fields", row->refname);
          for (i = 0; i < row->nfields; i++)
          {
            if (row->fieldnames[i])
              printf(" %s=var %d", row->fieldnames[i],
                   row->varnos[i]);
          }
          printf("\n");
        }
        break;
      case PLPGSQL_DTYPE_REC:
        append_string( dump.output, "<record name=\"%s\"/>", ((PLpgSQL_rec *) d)->refname ); 
        break;
      case PLPGSQL_DTYPE_RECFIELD:
        printf("RECFIELD %-16s of REC %d\n",
             ((PLpgSQL_recfield *) d)->fieldname,
             ((PLpgSQL_recfield *) d)->recparentno);
        break;
      case PLPGSQL_DTYPE_ARRAYELEM:
        printf("ARRAYELEM of VAR %d subscript ",
             ((PLpgSQL_arrayelem *) d)->arrayparentno);
        dump_expr( dump, ((PLpgSQL_arrayelem *) d)->subscript);
        printf("\n");
        break;
      case PLPGSQL_DTYPE_TRIGARG:
        printf("TRIGARG ");
        dump_expr( dump, ((PLpgSQL_trigarg *) d)->argnum);
        printf("\n");
        break;
      default:
        append_string( dump.output, "<entry type=\"unknown\"/>");
        printf("??? unknown data type %d\n", d->dtype);
    }
    append_string( dump.output, "</entry>" );
  }
  append_string( dump.output, "</data_area>" ); 

  dump_block( dump, func->action);
  append_string( dump.output, "</%s>", ROOTNODENAME ); 
  return *dump.output;
}

static void dump_parse_node( FunctionDumpContext dump, Node * node );


static void dump_parse_node( FunctionDumpContext dump, Node * node ) {
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
      append_string( dump.output, "<node tag=\"%d\"/>", nodeTag( node ) );
      break;


  }
}

/** taken from pl_funcs.c */

static void
dump_stmt( FunctionDumpContext dump, PLpgSQL_stmt *stmt)
{
  append_string( dump.output, "<statement line=\"%d\">", stmt->lineno );
  switch (stmt->cmd_type)
  {
    case PLPGSQL_STMT_BLOCK:
      dump_block( dump, (PLpgSQL_stmt_block *) stmt);
      break;
    case PLPGSQL_STMT_ASSIGN:
      dump_assign( dump, (PLpgSQL_stmt_assign *) stmt);
      break;
    case PLPGSQL_STMT_IF:
      dump_if( dump, (PLpgSQL_stmt_if *) stmt);
      break;
    case PLPGSQL_STMT_LOOP:
      dump_loop( dump, (PLpgSQL_stmt_loop *) stmt);
      break;
    case PLPGSQL_STMT_WHILE:
      dump_while( dump, (PLpgSQL_stmt_while *) stmt);
      break;
    case PLPGSQL_STMT_FORI:
      dump_fori( dump, (PLpgSQL_stmt_fori *) stmt);
      break;
    case PLPGSQL_STMT_FORS:
      dump_fors( dump, (PLpgSQL_stmt_fors *) stmt);
      break;
    case PLPGSQL_STMT_EXIT:
      dump_exit( dump, (PLpgSQL_stmt_exit *) stmt);
      break;
    case PLPGSQL_STMT_RETURN:
      dump_return( dump, (PLpgSQL_stmt_return *) stmt);
      break;
    case PLPGSQL_STMT_RETURN_NEXT:
      dump_return_next( dump, (PLpgSQL_stmt_return_next *) stmt);
      break;
    case PLPGSQL_STMT_RETURN_QUERY:
      dump_return_query( dump, (PLpgSQL_stmt_return_query *) stmt);
      break;
    case PLPGSQL_STMT_RAISE:
      dump_raise( dump, (PLpgSQL_stmt_raise *) stmt);
      break;
    case PLPGSQL_STMT_EXECSQL:
      dump_execsql( dump, (PLpgSQL_stmt_execsql *) stmt);
      break;
    case PLPGSQL_STMT_DYNEXECUTE:
      dump_dynexecute( dump, (PLpgSQL_stmt_dynexecute *) stmt);
      break;
    case PLPGSQL_STMT_DYNFORS:
      dump_dynfors( dump, (PLpgSQL_stmt_dynfors *) stmt);
      break;
    case PLPGSQL_STMT_GETDIAG:
      dump_getdiag( dump, (PLpgSQL_stmt_getdiag *) stmt);
      break;
    case PLPGSQL_STMT_OPEN:
      dump_open( dump, (PLpgSQL_stmt_open *) stmt);
      break;
    case PLPGSQL_STMT_FETCH:
      dump_fetch( dump, (PLpgSQL_stmt_fetch *) stmt);
      break;
    case PLPGSQL_STMT_CLOSE:
      dump_close( dump, (PLpgSQL_stmt_close *) stmt);
      break;
    case PLPGSQL_STMT_PERFORM:
      dump_perform( dump, (PLpgSQL_stmt_perform *) stmt);
      break;
    default:
      elog(ERROR, "unrecognized cmd_type: %d", stmt->cmd_type);
      break;
  }
  append_string( dump.output, "</statement>" );
}

static void
dump_stmts( FunctionDumpContext dump, List *stmts)
{
  append_string( dump.output, "<statements>" );
  ListCell   *s;

  foreach(s, stmts)
    dump_stmt( dump, (PLpgSQL_stmt *) lfirst(s));
  append_string( dump.output, "</statements>" );
}

static void
dump_block( FunctionDumpContext dump, PLpgSQL_stmt_block *block)
{
  append_string( dump.output, "<block name=\"%s\">", block->label == NULL ? "" : block->label );

  dump_stmts( dump, block->body);

  if (block->exceptions)
  {
    append_string( dump.output, "<exceptions>" );
    ListCell   *e;

    foreach(e, block->exceptions->exc_list)
    {
      append_string( dump.output, "<exception>" );
      PLpgSQL_exception *exc = (PLpgSQL_exception *) lfirst(e);
      PLpgSQL_condition *cond;

      printf("    EXCEPTION WHEN ");
      for (cond = exc->conditions; cond; cond = cond->next)
      {
        if (cond != exc->conditions)
          printf(" OR ");
        printf("%s", cond->condname);
      }
      printf(" THEN\n");
      dump_stmts( dump, exc->action);
      append_string( dump.output, "</exception>" );
    }
    append_string( dump.output, "</exceptions>" );
  }

  append_string( dump.output, "</block>" );
}

static void
dump_assign( FunctionDumpContext dump, PLpgSQL_stmt_assign *stmt)
{
  append_string( dump.output, "<assignment var=\"%d\">", stmt->varno );
  dump_expr( dump, stmt->expr);
  append_string( dump.output, "</assignment>" );
}

static void
dump_if( FunctionDumpContext dump, PLpgSQL_stmt_if *stmt)
{
  append_string( dump.output, "<if>" );
  append_string( dump.output, "<condition>" );
  dump_expr( dump, stmt->cond);
  append_string( dump.output, "</condition>" );

  append_string( dump.output, "<then>" );
  dump_stmts( dump, stmt->true_body);
  append_string( dump.output, "</then>" );

  if (stmt->false_body != NIL)
  {
    append_string( dump.output, "<else>" );
    dump_stmts( dump, stmt->false_body);
    append_string( dump.output, "</else>" );
  }

  append_string( dump.output, "</if>" );
}

static void
dump_loop( FunctionDumpContext dump, PLpgSQL_stmt_loop *stmt)
{
  append_string( dump.output, "<loop>" );
  dump_stmts( dump, stmt->body);
  append_string( dump.output, "</loop>" );
}

static void
dump_while( FunctionDumpContext dump, PLpgSQL_stmt_while *stmt)
{
  append_string( dump.output, "<while>" );
  append_string( dump.output, "<condition>" );
  dump_expr( dump, stmt->cond);
  append_string( dump.output, "</condition>" );
  dump_stmts( dump, stmt->body);
  append_string( dump.output, "</while>" );
}

static void
dump_fori( FunctionDumpContext dump, PLpgSQL_stmt_fori *stmt)
{
  append_string( dump.output, "<fori>" );
  printf("FORI %s %s\n", stmt->var->refname, (stmt->reverse) ? "REVERSE" : "NORMAL");

  printf("    lower = ");
  dump_expr( dump, stmt->lower);
  printf("\n");
  printf("    upper = ");
  dump_expr( dump, stmt->upper);
  printf("\n");
  printf("    step = ");
  dump_expr( dump, stmt->step);
  printf("\n");

  dump_stmts( dump, stmt->body);

  printf("    ENDFORI\n");
  append_string( dump.output, "</fori>" );
}

static void
dump_fors( FunctionDumpContext dump, PLpgSQL_stmt_fors *stmt)
{
  append_string( dump.output, "<fors>" );
  printf("FORS %s ", (stmt->rec != NULL) ? stmt->rec->refname : stmt->row->refname);
  dump_expr( dump, stmt->query);
  printf("\n");

  dump_stmts( dump, stmt->body);

  printf("    ENDFORS\n");
  append_string( dump.output, "</fors>" );
}

static void
dump_open( FunctionDumpContext dump, PLpgSQL_stmt_open *stmt)
{
  append_string( dump.output, "<open/>" );
  printf("OPEN curvar=%d\n", stmt->curvar);

  if (stmt->argquery != NULL)
  {
    printf("  arguments = '");
    dump_expr( dump, stmt->argquery);
    printf("'\n");
  }
  if (stmt->query != NULL)
  {
    printf("  query = '");
    dump_expr( dump, stmt->query);
    printf("'\n");
  }
  if (stmt->dynquery != NULL)
  {
    printf("  execute = '");
    dump_expr( dump, stmt->dynquery);
    printf("'\n");
  }

}

static void
dump_fetch( FunctionDumpContext dump, PLpgSQL_stmt_fetch *stmt)
{
  append_string( dump.output, "<fetch>" );

  if (!stmt->is_move)
  {
    printf("FETCH curvar=%d\n", stmt->curvar);
    dump_cursor_direction( dump, stmt);

    if (stmt->rec != NULL)
    {
//      printf("    target = %d %s\n", stmt->rec->recno, stmt->rec->refname);
    }
    if (stmt->row != NULL)
    {
//      printf("    target = %d %s\n", stmt->row->rowno, stmt->row->refname);
    }
  }
  else
  {
    printf("MOVE curvar=%d\n", stmt->curvar);
    dump_cursor_direction( dump, stmt);
  }
  append_string( dump.output, "</fetch>" );
}

static void
dump_cursor_direction( FunctionDumpContext dump, PLpgSQL_stmt_fetch *stmt)
{
  switch (stmt->direction)
  {
    case FETCH_FORWARD:
      printf("    FORWARD ");
      break;
    case FETCH_BACKWARD:
      printf("    BACKWARD ");
      break;
    case FETCH_ABSOLUTE:
      printf("    ABSOLUTE ");
      break;
    case FETCH_RELATIVE:
      printf("    RELATIVE ");
      break;
    default:
      printf("??? unknown cursor direction %d", stmt->direction);
  }

  if (stmt->expr)
  {
    dump_expr( dump, stmt->expr);
    printf("\n");
  }
  else
    printf("%d\n", stmt->how_many);

}

static void
dump_close( FunctionDumpContext dump, PLpgSQL_stmt_close *stmt)
{
  append_string( dump.output, "<close/>" );
  printf("CLOSE curvar=%d\n", stmt->curvar);
}

static void
dump_perform( FunctionDumpContext dump, PLpgSQL_stmt_perform *stmt)
{
  append_string( dump.output, "<perform>" );
  printf("PERFORM expr = ");
  dump_expr( dump, stmt->expr);
  printf("\n");
  append_string( dump.output, "</perform>" );
}

static void
dump_exit( FunctionDumpContext dump, PLpgSQL_stmt_exit *stmt)
{
  append_string( dump.output, "<exit>" );
  printf("%s", stmt->is_exit ? "EXIT" : "CONTINUE");
  if (stmt->label != NULL)
    printf(" label='%s'", stmt->label);
  if (stmt->cond != NULL)
  {
    printf(" WHEN ");
    dump_expr( dump, stmt->cond);
  }
  printf("\n");
  append_string( dump.output, "</exit>" );
}

static void
dump_return( FunctionDumpContext dump, PLpgSQL_stmt_return *stmt)
{
  append_string( dump.output, "<return>" );
  printf("RETURN ");
  if (stmt->retvarno >= 0)
    printf("variable %d", stmt->retvarno);
  else if (stmt->expr != NULL)
    dump_expr( dump, stmt->expr);
  else
    printf("NULL");
  printf("\n");
  append_string( dump.output, "</return>" );
}

static void
dump_return_next( FunctionDumpContext dump, PLpgSQL_stmt_return_next *stmt)
{
  append_string( dump.output, "<return_next>" );
  printf("RETURN NEXT ");
  if (stmt->retvarno >= 0)
    printf("variable %d", stmt->retvarno);
  else if (stmt->expr != NULL)
    dump_expr( dump, stmt->expr);
  else
    printf("NULL");
  printf("\n");
  append_string( dump.output, "</return_next>" );
}

static void
dump_return_query( FunctionDumpContext dump, PLpgSQL_stmt_return_query *stmt)
{
  append_string( dump.output, "<return_query>" );
  printf("RETURN QUERY ");
  dump_expr( dump, stmt->query);
  printf("\n");
  append_string( dump.output, "</return_query>" );
}

static void
dump_raise( FunctionDumpContext dump, PLpgSQL_stmt_raise *stmt)
{
  append_string( dump.output, "<raise>" );
  ListCell   *lc;
  int      i = 0;

  printf("RAISE '%s'\n", stmt->message);
  foreach(lc, stmt->params)
  {
    printf("    parameter %d: ", i++);
    dump_expr( dump, (PLpgSQL_expr *) lfirst(lc));
    printf("\n");
  }
  append_string( dump.output, "</raise>" );
}

static void
dump_execsql( FunctionDumpContext dump, PLpgSQL_stmt_execsql *stmt)
{
  append_string( dump.output, "<execsql>" );
  printf("EXECSQL ");
  dump_expr( dump, stmt->sqlstmt);
  printf("\n");

  if (stmt->rec != NULL)
  {
//    printf("    INTO%s target = %d %s\n",
//         stmt->strict ? " STRICT" : "",
//         stmt->rec->recno, stmt->rec->refname);
  }
  if (stmt->row != NULL)
  {
//    printf("    INTO%s target = %d %s\n",
//         stmt->strict ? " STRICT" : "",
//         stmt->row->rowno, stmt->row->refname);
  }
  append_string( dump.output, "</execsql>" );
}

static void
dump_dynexecute( FunctionDumpContext dump, PLpgSQL_stmt_dynexecute *stmt)
{
  append_string( dump.output, "<execute>" );
  dump_expr( dump, stmt->query);

  if (stmt->rec != NULL)
  {
//    printf("    INTO%s target = %d %s\n",
//         stmt->strict ? " STRICT" : "",
//         stmt->rec->recno, stmt->rec->refname);
  }
  if (stmt->row != NULL)
  {
//    printf("    INTO%s target = %d %s\n",
//         stmt->strict ? " STRICT" : "",
//         stmt->row->rowno, stmt->row->refname);
  }
  append_string( dump.output, "</execute>" );
}

static void
dump_dynfors( FunctionDumpContext dump, PLpgSQL_stmt_dynfors *stmt)
{
  append_string( dump.output, "<dynfors>" );
  printf("FORS %s EXECUTE ", (stmt->rec != NULL) ? stmt->rec->refname : stmt->row->refname);
  dump_expr( dump, stmt->query);
  printf("\n");

  dump_stmts( dump, stmt->body);

  printf("    ENDFORS\n");
  append_string( dump.output, "</dynfors>" );
}

static void
dump_getdiag( FunctionDumpContext dump, PLpgSQL_stmt_getdiag *stmt)
{
  append_string( dump.output, "<getdiag>" );
  ListCell   *lc;

  printf("GET DIAGNOSTICS ");
  foreach(lc, stmt->diag_items)
  {
    PLpgSQL_diag_item *diag_item = (PLpgSQL_diag_item *) lfirst(lc);

    if (lc != list_head(stmt->diag_items))
      printf(", ");

    printf("{var %d} = ", diag_item->target);

    switch (diag_item->kind)
    {
      case PLPGSQL_GETDIAG_ROW_COUNT:
        printf("ROW_COUNT");
        break;

      case PLPGSQL_GETDIAG_RESULT_OID:
        printf("RESULT_OID");
        break;

      default:
        printf("???");
        break;
    }
  }
  printf("\n");
  append_string( dump.output, "<getdiag>" );
}

static void
dump_expr( FunctionDumpContext dump, PLpgSQL_expr *expr )
{
  int i;
  Oid * paramTypes;

  append_string( dump.output, "<expression params=\"%d\">", expr->nparams );
  append_string( dump.output, "<query>%s</query>", expr->query );
  if (expr->nparams > 0)
  {
    append_string( dump.output, "<parameters>" );
    paramTypes = (Oid *) palloc( expr->nparams * sizeof(Oid) );
    for (i = 0; i < expr->nparams; i++)
    {
      PLpgSQL_datum *datum;
      datum = dump.func->datums[expr->params[i]];
      paramTypes[i] = ((PLpgSQL_var *)dump.func->datums[expr->params[i]])->datatype->typoid;
      append_string( dump.output, "<param outer=\"%d\" inner=\"%d\"/>", i + 1, expr->params[i] );
    }
    append_string( dump.output, "</parameters>" );
  } else {
    paramTypes = NULL;
  }
  append_string( dump.output, "<parse_tree>%s</parse_tree>", dump_sql_parse_tree_internal( expr->query ) );
  append_string( dump.output, "</expression>" );
}

