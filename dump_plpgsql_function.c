
#define _GNU_SOURCE
#include <stdio.h>

#include <postgres.h>
#include <fmgr.h>
#include <plpgsql.h>

#define BUFSIZE 4096

#define ROOTNODENAME "function_tree"

PG_MODULE_MAGIC;

/*
 * CREATE OR REPLACE FUNCTION dump_plpgsql_function(text) returns text AS '/home/sven/diplom/dump_plpgsql_function/dump_plpgsql_function.so', 'dump_plpgsql_function' LANGUAGE C STRICT;
 *
 */

static int	dump_indent;

static void dump_ind(void);
static void dump_stmt(PLpgSQL_stmt *stmt);
static void dump_block(PLpgSQL_stmt_block *block);
static void dump_assign(PLpgSQL_stmt_assign *stmt);
static void dump_if(PLpgSQL_stmt_if *stmt);
static void dump_loop(PLpgSQL_stmt_loop *stmt);
static void dump_while(PLpgSQL_stmt_while *stmt);
static void dump_fori(PLpgSQL_stmt_fori *stmt);
static void dump_fors(PLpgSQL_stmt_fors *stmt);
static void dump_exit(PLpgSQL_stmt_exit *stmt);
static void dump_return(PLpgSQL_stmt_return *stmt);
static void dump_return_next(PLpgSQL_stmt_return_next *stmt);
static void dump_return_query(PLpgSQL_stmt_return_query *stmt);
static void dump_raise(PLpgSQL_stmt_raise *stmt);
static void dump_execsql(PLpgSQL_stmt_execsql *stmt);
static void dump_dynexecute(PLpgSQL_stmt_dynexecute *stmt);
static void dump_dynfors(PLpgSQL_stmt_dynfors *stmt);
static void dump_getdiag(PLpgSQL_stmt_getdiag *stmt);
static void dump_open(PLpgSQL_stmt_open *stmt);
static void dump_fetch(PLpgSQL_stmt_fetch *stmt);
static void dump_cursor_direction(PLpgSQL_stmt_fetch *stmt);
static void dump_close(PLpgSQL_stmt_close *stmt);
static void dump_perform(PLpgSQL_stmt_perform *stmt);
static void dump_expr(PLpgSQL_expr *expr);



text * cstring_to_text_with_len( const char * s, int len ) {
  text * result = (text *) palloc(len + VARHDRSZ);
  SET_VARSIZE(result, len + VARHDRSZ);
  memcpy( VARDATA(result), s, len );
  return result;
}

text * cstring_to_text( const char * s ) {
  return cstring_to_text_with_len( s, strlen(s));
}

PG_FUNCTION_INFO_V1(dump_plpgsql_function);

static char * dumptree(PLpgSQL_function *func);

Datum dump_plpgsql_function( PG_FUNCTION_ARGS )
{
  text * function_name = PG_GETARG_TEXT_P( 0 );

  int funcoid = 32770;


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
 
  char * result = (char *) palloc( BUFSIZE + 1);
  memset( result, 0, BUFSIZE + 1 );
  int offset = 0;
  offset += snprintf( result + offset, BUFSIZE > offset ? BUFSIZE - offset : 0, "Name: %s\n", func->fn_name );
//  offset += snprintf( result + offset, BUFSIZE > offset ? BUFSIZE - offset : 0, "Action: %x\n", func->action->body );
  ListCell *s;
  foreach( s, func->action->body )
    offset += snprintf( result + offset, BUFSIZE > offset ? BUFSIZE - offset : 0, "%3d:\n", ((PLpgSQL_stmt *) lfirst(s))->lineno );

  result = dumptree( func );

  PG_RETURN_TEXT_P( cstring_to_text( result ) );
}

static int 
__attribute__ ((format (printf, 2, 3)))
append_string( char ** buffer, char * fmt, ... )
{
  va_list ap;
  char * tmp;

  va_start(ap, fmt);
  if ( vasprintf(&tmp, fmt, ap) == -1 ) {
    /* error */
    return -1;
  } else {
    size_t offset = 0;
    if ( !*buffer ) {
      *buffer = (char *) palloc( strlen( tmp ) + 1 );
    } else {
      offset = strlen( *buffer );
      *buffer = (char *) repalloc( *buffer, offset + strlen( tmp ) + 1 );
    }
    if (!*buffer) {
      free( tmp );
      return -1;
    } else {
      memcpy( *buffer + offset, tmp, strlen(tmp) + 1 );
      free( tmp );
    }
    return 0;
  }
}


static char * dumptree(PLpgSQL_function *func)
{
  char * result = NULL;
	int			i;
	PLpgSQL_datum *d;

	printf("\nExecution tree of successfully compiled PL/pgSQL function %s:\n",
		   func->fn_name);

  append_string( &result, "<%s function_name=\"%s\">\n", ROOTNODENAME, func->fn_name ); 

	printf("\nFunction's data area:\n");
  append_string( &result, "<data_area>\n" ); 
	for (i = 0; i < func->ndatums; i++)
	{
		d = func->datums[i];

    append_string( &result, "<entry index=\"%d\">\n", i );
		printf("    entry %d: ", i);
		switch (d->dtype)
		{
			case PLPGSQL_DTYPE_VAR:
				{
					PLpgSQL_var *var = (PLpgSQL_var *) d;

          append_string( &result, "<variable name=\"%s\" datatype=\"%s\" datatype_oid=\"%u\" atttypmod=\"%d\"/>\n",
              var->refname, var->datatype->typname, 
              var->datatype->typoid, var->datatype->atttypmod );
					printf("VAR %-16s type %s (typoid %u) atttypmod %d\n",
						   var->refname, var->datatype->typname,
						   var->datatype->typoid,
						   var->datatype->atttypmod);
					if (var->isconst)
						printf("                                  CONSTANT\n");
					if (var->notnull)
						printf("                                  NOT NULL\n");
					if (var->default_val != NULL)
					{
						printf("                                  DEFAULT ");
						dump_expr(var->default_val);
						printf("\n");
					}
					if (var->cursor_explicit_expr != NULL)
					{
						if (var->cursor_explicit_argrow >= 0)
							printf("                                  CURSOR argument row %d\n", var->cursor_explicit_argrow);

						printf("                                  CURSOR IS ");
						dump_expr(var->cursor_explicit_expr);
						printf("\n");
					}
				}
				break;
			case PLPGSQL_DTYPE_ROW:
				{
					PLpgSQL_row *row = (PLpgSQL_row *) d;
					int			i;

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
				printf("REC %s\n", ((PLpgSQL_rec *) d)->refname);
				break;
			case PLPGSQL_DTYPE_RECFIELD:
				printf("RECFIELD %-16s of REC %d\n",
					   ((PLpgSQL_recfield *) d)->fieldname,
					   ((PLpgSQL_recfield *) d)->recparentno);
				break;
			case PLPGSQL_DTYPE_ARRAYELEM:
				printf("ARRAYELEM of VAR %d subscript ",
					   ((PLpgSQL_arrayelem *) d)->arrayparentno);
				dump_expr(((PLpgSQL_arrayelem *) d)->subscript);
				printf("\n");
				break;
			case PLPGSQL_DTYPE_TRIGARG:
				printf("TRIGARG ");
				dump_expr(((PLpgSQL_trigarg *) d)->argnum);
				printf("\n");
				break;
			default:
        append_string( &result, "<entry type=\"unknown\"/>\n");
				printf("??? unknown data type %d\n", d->dtype);
		}
    append_string( &result, "</entry>\n" );
	}
  append_string( &result, "</data_area>\n" ); 

	printf("\nFunction's statements:\n");

	dump_indent = 0;
	printf("%3d:", func->action->lineno);
	dump_block(func->action);
	printf("\nEnd of execution tree of function %s\n\n", func->fn_name);
	fflush(stdout);
  append_string( &result, "</%s>", ROOTNODENAME ); 
  return result;
}

/** taken from pl_funcs.c */

static void
dump_ind(void)
{
	int			i;

	for (i = 0; i < dump_indent; i++)
		printf(" ");
}

static void
dump_stmt(PLpgSQL_stmt *stmt)
{
	printf("%3d:", stmt->lineno);
	switch (stmt->cmd_type)
	{
		case PLPGSQL_STMT_BLOCK:
			dump_block((PLpgSQL_stmt_block *) stmt);
			break;
		case PLPGSQL_STMT_ASSIGN:
			dump_assign((PLpgSQL_stmt_assign *) stmt);
			break;
		case PLPGSQL_STMT_IF:
			dump_if((PLpgSQL_stmt_if *) stmt);
			break;
		case PLPGSQL_STMT_LOOP:
			dump_loop((PLpgSQL_stmt_loop *) stmt);
			break;
		case PLPGSQL_STMT_WHILE:
			dump_while((PLpgSQL_stmt_while *) stmt);
			break;
		case PLPGSQL_STMT_FORI:
			dump_fori((PLpgSQL_stmt_fori *) stmt);
			break;
		case PLPGSQL_STMT_FORS:
			dump_fors((PLpgSQL_stmt_fors *) stmt);
			break;
		case PLPGSQL_STMT_EXIT:
			dump_exit((PLpgSQL_stmt_exit *) stmt);
			break;
		case PLPGSQL_STMT_RETURN:
			dump_return((PLpgSQL_stmt_return *) stmt);
			break;
		case PLPGSQL_STMT_RETURN_NEXT:
			dump_return_next((PLpgSQL_stmt_return_next *) stmt);
			break;
		case PLPGSQL_STMT_RETURN_QUERY:
			dump_return_query((PLpgSQL_stmt_return_query *) stmt);
			break;
		case PLPGSQL_STMT_RAISE:
			dump_raise((PLpgSQL_stmt_raise *) stmt);
			break;
		case PLPGSQL_STMT_EXECSQL:
			dump_execsql((PLpgSQL_stmt_execsql *) stmt);
			break;
		case PLPGSQL_STMT_DYNEXECUTE:
			dump_dynexecute((PLpgSQL_stmt_dynexecute *) stmt);
			break;
		case PLPGSQL_STMT_DYNFORS:
			dump_dynfors((PLpgSQL_stmt_dynfors *) stmt);
			break;
		case PLPGSQL_STMT_GETDIAG:
			dump_getdiag((PLpgSQL_stmt_getdiag *) stmt);
			break;
		case PLPGSQL_STMT_OPEN:
			dump_open((PLpgSQL_stmt_open *) stmt);
			break;
		case PLPGSQL_STMT_FETCH:
			dump_fetch((PLpgSQL_stmt_fetch *) stmt);
			break;
		case PLPGSQL_STMT_CLOSE:
			dump_close((PLpgSQL_stmt_close *) stmt);
			break;
		case PLPGSQL_STMT_PERFORM:
			dump_perform((PLpgSQL_stmt_perform *) stmt);
			break;
		default:
			elog(ERROR, "unrecognized cmd_type: %d", stmt->cmd_type);
			break;
	}
}

static void
dump_stmts(List *stmts)
{
	ListCell   *s;

	dump_indent += 2;
	foreach(s, stmts)
		dump_stmt((PLpgSQL_stmt *) lfirst(s));
	dump_indent -= 2;
}

static void
dump_block(PLpgSQL_stmt_block *block)
{
	char	   *name;

	if (block->label == NULL)
		name = "*unnamed*";
	else
		name = block->label;

	dump_ind();
	printf("BLOCK <<%s>>\n", name);

	dump_stmts(block->body);

	if (block->exceptions)
	{
		ListCell   *e;

		foreach(e, block->exceptions->exc_list)
		{
			PLpgSQL_exception *exc = (PLpgSQL_exception *) lfirst(e);
			PLpgSQL_condition *cond;

			dump_ind();
			printf("    EXCEPTION WHEN ");
			for (cond = exc->conditions; cond; cond = cond->next)
			{
				if (cond != exc->conditions)
					printf(" OR ");
				printf("%s", cond->condname);
			}
			printf(" THEN\n");
			dump_stmts(exc->action);
		}
	}

	dump_ind();
	printf("    END -- %s\n", name);
}

static void
dump_assign(PLpgSQL_stmt_assign *stmt)
{
	dump_ind();
	printf("ASSIGN var %d := ", stmt->varno);
	dump_expr(stmt->expr);
	printf("\n");
}

static void
dump_if(PLpgSQL_stmt_if *stmt)
{
	dump_ind();
	printf("IF ");
	dump_expr(stmt->cond);
	printf(" THEN\n");

	dump_stmts(stmt->true_body);

	if (stmt->false_body != NIL)
	{
		dump_ind();
		printf("    ELSE\n");
		dump_stmts(stmt->false_body);
	}

	dump_ind();
	printf("    ENDIF\n");
}

static void
dump_loop(PLpgSQL_stmt_loop *stmt)
{
	dump_ind();
	printf("LOOP\n");

	dump_stmts(stmt->body);

	dump_ind();
	printf("    ENDLOOP\n");
}

static void
dump_while(PLpgSQL_stmt_while *stmt)
{
	dump_ind();
	printf("WHILE ");
	dump_expr(stmt->cond);
	printf("\n");

	dump_stmts(stmt->body);

	dump_ind();
	printf("    ENDWHILE\n");
}

static void
dump_fori(PLpgSQL_stmt_fori *stmt)
{
	dump_ind();
	printf("FORI %s %s\n", stmt->var->refname, (stmt->reverse) ? "REVERSE" : "NORMAL");

	dump_indent += 2;
	dump_ind();
	printf("    lower = ");
	dump_expr(stmt->lower);
	printf("\n");
	dump_ind();
	printf("    upper = ");
	dump_expr(stmt->upper);
	printf("\n");
	dump_ind();
	printf("    step = ");
	dump_expr(stmt->step);
	printf("\n");
	dump_indent -= 2;

	dump_stmts(stmt->body);

	dump_ind();
	printf("    ENDFORI\n");
}

static void
dump_fors(PLpgSQL_stmt_fors *stmt)
{
	dump_ind();
	printf("FORS %s ", (stmt->rec != NULL) ? stmt->rec->refname : stmt->row->refname);
	dump_expr(stmt->query);
	printf("\n");

	dump_stmts(stmt->body);

	dump_ind();
	printf("    ENDFORS\n");
}

static void
dump_open(PLpgSQL_stmt_open *stmt)
{
	dump_ind();
	printf("OPEN curvar=%d\n", stmt->curvar);

	dump_indent += 2;
	if (stmt->argquery != NULL)
	{
		dump_ind();
		printf("  arguments = '");
		dump_expr(stmt->argquery);
		printf("'\n");
	}
	if (stmt->query != NULL)
	{
		dump_ind();
		printf("  query = '");
		dump_expr(stmt->query);
		printf("'\n");
	}
	if (stmt->dynquery != NULL)
	{
		dump_ind();
		printf("  execute = '");
		dump_expr(stmt->dynquery);
		printf("'\n");
	}
	dump_indent -= 2;

}

static void
dump_fetch(PLpgSQL_stmt_fetch *stmt)
{
	dump_ind();

	if (!stmt->is_move)
	{
		printf("FETCH curvar=%d\n", stmt->curvar);
		dump_cursor_direction(stmt);

		dump_indent += 2;
		if (stmt->rec != NULL)
		{
			dump_ind();
			printf("    target = %d %s\n", stmt->rec->recno, stmt->rec->refname);
		}
		if (stmt->row != NULL)
		{
			dump_ind();
			printf("    target = %d %s\n", stmt->row->rowno, stmt->row->refname);
		}
		dump_indent -= 2;
	}
	else
	{
		printf("MOVE curvar=%d\n", stmt->curvar);
		dump_cursor_direction(stmt);
	}
}

static void
dump_cursor_direction(PLpgSQL_stmt_fetch *stmt)
{
	dump_indent += 2;
	dump_ind();
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
		dump_expr(stmt->expr);
		printf("\n");
	}
	else
		printf("%d\n", stmt->how_many);

	dump_indent -= 2;
}

static void
dump_close(PLpgSQL_stmt_close *stmt)
{
	dump_ind();
	printf("CLOSE curvar=%d\n", stmt->curvar);
}

static void
dump_perform(PLpgSQL_stmt_perform *stmt)
{
	dump_ind();
	printf("PERFORM expr = ");
	dump_expr(stmt->expr);
	printf("\n");
}

static void
dump_exit(PLpgSQL_stmt_exit *stmt)
{
	dump_ind();
	printf("%s", stmt->is_exit ? "EXIT" : "CONTINUE");
	if (stmt->label != NULL)
		printf(" label='%s'", stmt->label);
	if (stmt->cond != NULL)
	{
		printf(" WHEN ");
		dump_expr(stmt->cond);
	}
	printf("\n");
}

static void
dump_return(PLpgSQL_stmt_return *stmt)
{
	dump_ind();
	printf("RETURN ");
	if (stmt->retvarno >= 0)
		printf("variable %d", stmt->retvarno);
	else if (stmt->expr != NULL)
		dump_expr(stmt->expr);
	else
		printf("NULL");
	printf("\n");
}

static void
dump_return_next(PLpgSQL_stmt_return_next *stmt)
{
	dump_ind();
	printf("RETURN NEXT ");
	if (stmt->retvarno >= 0)
		printf("variable %d", stmt->retvarno);
	else if (stmt->expr != NULL)
		dump_expr(stmt->expr);
	else
		printf("NULL");
	printf("\n");
}

static void
dump_return_query(PLpgSQL_stmt_return_query *stmt)
{
	dump_ind();
	printf("RETURN QUERY ");
	dump_expr(stmt->query);
	printf("\n");
}

static void
dump_raise(PLpgSQL_stmt_raise *stmt)
{
	ListCell   *lc;
	int			i = 0;

	dump_ind();
	printf("RAISE '%s'\n", stmt->message);
	dump_indent += 2;
	foreach(lc, stmt->params)
	{
		dump_ind();
		printf("    parameter %d: ", i++);
		dump_expr((PLpgSQL_expr *) lfirst(lc));
		printf("\n");
	}
	dump_indent -= 2;
}

static void
dump_execsql(PLpgSQL_stmt_execsql *stmt)
{
	dump_ind();
	printf("EXECSQL ");
	dump_expr(stmt->sqlstmt);
	printf("\n");

	dump_indent += 2;
	if (stmt->rec != NULL)
	{
		dump_ind();
		printf("    INTO%s target = %d %s\n",
			   stmt->strict ? " STRICT" : "",
			   stmt->rec->recno, stmt->rec->refname);
	}
	if (stmt->row != NULL)
	{
		dump_ind();
		printf("    INTO%s target = %d %s\n",
			   stmt->strict ? " STRICT" : "",
			   stmt->row->rowno, stmt->row->refname);
	}
	dump_indent -= 2;
}

static void
dump_dynexecute(PLpgSQL_stmt_dynexecute *stmt)
{
	dump_ind();
	printf("EXECUTE ");
	dump_expr(stmt->query);
	printf("\n");

	dump_indent += 2;
	if (stmt->rec != NULL)
	{
		dump_ind();
		printf("    INTO%s target = %d %s\n",
			   stmt->strict ? " STRICT" : "",
			   stmt->rec->recno, stmt->rec->refname);
	}
	if (stmt->row != NULL)
	{
		dump_ind();
		printf("    INTO%s target = %d %s\n",
			   stmt->strict ? " STRICT" : "",
			   stmt->row->rowno, stmt->row->refname);
	}
	dump_indent -= 2;
}

static void
dump_dynfors(PLpgSQL_stmt_dynfors *stmt)
{
	dump_ind();
	printf("FORS %s EXECUTE ", (stmt->rec != NULL) ? stmt->rec->refname : stmt->row->refname);
	dump_expr(stmt->query);
	printf("\n");

	dump_stmts(stmt->body);

	dump_ind();
	printf("    ENDFORS\n");
}

static void
dump_getdiag(PLpgSQL_stmt_getdiag *stmt)
{
	ListCell   *lc;

	dump_ind();
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
}

static void
dump_expr(PLpgSQL_expr *expr)
{
	int			i;

	printf("'%s", expr->query);
	if (expr->nparams > 0)
	{
		printf(" {");
		for (i = 0; i < expr->nparams; i++)
		{
			if (i > 0)
				printf(", ");
			printf("$%d=%d", i + 1, expr->params[i]);
		}
		printf("}");
	}
	printf("'");
}

