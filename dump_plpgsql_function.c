
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

static char * dumptree(PLpgSQL_function *func);
static void dump_stmt( char ** output, PLpgSQL_stmt *stmt);
static void dump_block( char ** output, PLpgSQL_stmt_block *block);
static void dump_loop( char ** output, PLpgSQL_stmt_loop *stmt);
static void dump_while( char ** output, PLpgSQL_stmt_while *stmt);
static void dump_fori( char ** output, PLpgSQL_stmt_fori *stmt);
static void dump_fors( char ** output, PLpgSQL_stmt_fors *stmt);
static void dump_exit( char ** output, PLpgSQL_stmt_exit *stmt);
static void dump_return( char ** output, PLpgSQL_stmt_return *stmt);
static void dump_return_next( char ** output, PLpgSQL_stmt_return_next *stmt);
static void dump_return_query( char ** output, PLpgSQL_stmt_return_query *stmt);
static void dump_raise( char ** output, PLpgSQL_stmt_raise *stmt);
static void dump_execsql( char ** output, PLpgSQL_stmt_execsql *stmt);
static void dump_dynexecute( char ** output, PLpgSQL_stmt_dynexecute *stmt);
static void dump_dynfors( char ** output, PLpgSQL_stmt_dynfors *stmt);
static void dump_getdiag( char ** output, PLpgSQL_stmt_getdiag *stmt);
static void dump_open( char ** output, PLpgSQL_stmt_open *stmt);
static void dump_fetch( char ** output, PLpgSQL_stmt_fetch *stmt);
static void dump_cursor_direction( char ** output, PLpgSQL_stmt_fetch *stmt);
static void dump_close( char ** output, PLpgSQL_stmt_close *stmt);
static void dump_perform( char ** output, PLpgSQL_stmt_perform *stmt);
static void dump_expr( char ** result, PLpgSQL_expr *expr );
static void dump_assign( char ** output, PLpgSQL_stmt_assign *stmt);
static void dump_if( char ** output, PLpgSQL_stmt_if *stmt);


/** helper function to convert c strings to text */
text * cstring_to_text_with_len( const char * s, int len ) {
  text * result = (text *) palloc(len + VARHDRSZ);
  SET_VARSIZE(result, len + VARHDRSZ);
  memcpy( VARDATA(result), s, len );
  return result;
}

/** helper function to convert c strings to text */
text * cstring_to_text( const char * s ) {
  return cstring_to_text_with_len( s, strlen(s));
}

/** helper function to append c string to existing c string */
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
  char ** output;
  *output = NULL;
	int			i;
	PLpgSQL_datum *d;

  append_string( output, "<%s function_name=\"%s\">", ROOTNODENAME, func->fn_name ); 

  append_string( output, "<data_area>" ); 
	for (i = 0; i < func->ndatums; i++)
	{
		d = func->datums[i];

    append_string( output, "<entry index=\"%d\">", i );
		switch (d->dtype)
		{
			case PLPGSQL_DTYPE_VAR:
				{
					PLpgSQL_var *var = (PLpgSQL_var *) d;

          append_string( output, "<variable name=\"%s\" datatype=\"%s\" datatype_oid=\"%u\" atttypmod=\"%d\" constant=\"%s\" not_null=\"%s\">",
              var->refname, var->datatype->typname, 
              var->datatype->typoid, var->datatype->atttypmod,
              var->isconst ? "yes" : "no", var->notnull ? "yes" : "no" );
					if (var->default_val != NULL)
					{
            append_string( output, "<default>" ); 
						dump_expr( output, var->default_val);
            append_string( output, "</default>" ); 
					}
					if (var->cursor_explicit_expr != NULL)
					{
						if (var->cursor_explicit_argrow >= 0)
							printf("                                  CURSOR argument row %d\n", var->cursor_explicit_argrow);

						printf("                                  CURSOR IS ");
						dump_expr( output, var->cursor_explicit_expr);
						printf("\n");
					}
				}
        append_string( output, "</variable>" ); 
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
        append_string( output, "<record name=\"%s\"/>", ((PLpgSQL_rec *) d)->refname ); 
				break;
			case PLPGSQL_DTYPE_RECFIELD:
				printf("RECFIELD %-16s of REC %d\n",
					   ((PLpgSQL_recfield *) d)->fieldname,
					   ((PLpgSQL_recfield *) d)->recparentno);
				break;
			case PLPGSQL_DTYPE_ARRAYELEM:
				printf("ARRAYELEM of VAR %d subscript ",
					   ((PLpgSQL_arrayelem *) d)->arrayparentno);
				dump_expr( output, ((PLpgSQL_arrayelem *) d)->subscript);
				printf("\n");
				break;
			case PLPGSQL_DTYPE_TRIGARG:
				printf("TRIGARG ");
				dump_expr( output, ((PLpgSQL_trigarg *) d)->argnum);
				printf("\n");
				break;
			default:
        append_string( output, "<entry type=\"unknown\"/>");
				printf("??? unknown data type %d\n", d->dtype);
		}
    append_string( output, "</entry>" );
	}
  append_string( output, "</data_area>" ); 

	dump_block( output, func->action);
  append_string( output, "</%s>", ROOTNODENAME ); 
  return *output;
}

/** taken from pl_funcs.c */

static void
dump_stmt( char ** output, PLpgSQL_stmt *stmt)
{
  append_string( output, "<statement line=\"%d\">", stmt->lineno );
	switch (stmt->cmd_type)
	{
		case PLPGSQL_STMT_BLOCK:
			dump_block( output, (PLpgSQL_stmt_block *) stmt);
			break;
		case PLPGSQL_STMT_ASSIGN:
			dump_assign( output, (PLpgSQL_stmt_assign *) stmt);
			break;
		case PLPGSQL_STMT_IF:
			dump_if( output, (PLpgSQL_stmt_if *) stmt);
			break;
		case PLPGSQL_STMT_LOOP:
			dump_loop( output, (PLpgSQL_stmt_loop *) stmt);
			break;
		case PLPGSQL_STMT_WHILE:
			dump_while( output, (PLpgSQL_stmt_while *) stmt);
			break;
		case PLPGSQL_STMT_FORI:
			dump_fori( output, (PLpgSQL_stmt_fori *) stmt);
			break;
		case PLPGSQL_STMT_FORS:
			dump_fors( output, (PLpgSQL_stmt_fors *) stmt);
			break;
		case PLPGSQL_STMT_EXIT:
			dump_exit( output, (PLpgSQL_stmt_exit *) stmt);
			break;
		case PLPGSQL_STMT_RETURN:
			dump_return( output, (PLpgSQL_stmt_return *) stmt);
			break;
		case PLPGSQL_STMT_RETURN_NEXT:
			dump_return_next( output, (PLpgSQL_stmt_return_next *) stmt);
			break;
		case PLPGSQL_STMT_RETURN_QUERY:
			dump_return_query( output, (PLpgSQL_stmt_return_query *) stmt);
			break;
		case PLPGSQL_STMT_RAISE:
			dump_raise( output, (PLpgSQL_stmt_raise *) stmt);
			break;
		case PLPGSQL_STMT_EXECSQL:
			dump_execsql( output, (PLpgSQL_stmt_execsql *) stmt);
			break;
		case PLPGSQL_STMT_DYNEXECUTE:
			dump_dynexecute( output, (PLpgSQL_stmt_dynexecute *) stmt);
			break;
		case PLPGSQL_STMT_DYNFORS:
			dump_dynfors( output, (PLpgSQL_stmt_dynfors *) stmt);
			break;
		case PLPGSQL_STMT_GETDIAG:
			dump_getdiag( output, (PLpgSQL_stmt_getdiag *) stmt);
			break;
		case PLPGSQL_STMT_OPEN:
			dump_open( output, (PLpgSQL_stmt_open *) stmt);
			break;
		case PLPGSQL_STMT_FETCH:
			dump_fetch( output, (PLpgSQL_stmt_fetch *) stmt);
			break;
		case PLPGSQL_STMT_CLOSE:
			dump_close( output, (PLpgSQL_stmt_close *) stmt);
			break;
		case PLPGSQL_STMT_PERFORM:
			dump_perform( output, (PLpgSQL_stmt_perform *) stmt);
			break;
		default:
			elog(ERROR, "unrecognized cmd_type: %d", stmt->cmd_type);
			break;
	}
  append_string( output, "</statement>" );
}

static void
dump_stmts( char ** output, List *stmts)
{
  append_string( output, "<statements>" );
	ListCell   *s;

	foreach(s, stmts)
		dump_stmt( output, (PLpgSQL_stmt *) lfirst(s));
  append_string( output, "</statements>" );
}

static void
dump_block( char ** output, PLpgSQL_stmt_block *block)
{
  append_string( output, "<block name=\"%s\">", block->label == NULL ? "" : block->label );

	dump_stmts( output, block->body);

	if (block->exceptions)
	{
    append_string( output, "<exceptions>" );
		ListCell   *e;

		foreach(e, block->exceptions->exc_list)
		{
      append_string( output, "<exception>" );
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
			dump_stmts( output, exc->action);
      append_string( output, "</exception>" );
		}
    append_string( output, "</exceptions>" );
	}

  append_string( output, "</block>" );
}

static void
dump_assign( char ** output, PLpgSQL_stmt_assign *stmt)
{
  append_string( output, "<assignment var=\"%d\">", stmt->varno );
	dump_expr( output, stmt->expr);
  append_string( output, "</assignment>" );
}

static void
dump_if( char ** output, PLpgSQL_stmt_if *stmt)
{
  append_string( output, "<if>" );
  append_string( output, "<condition>" );
	dump_expr( output, stmt->cond);
  append_string( output, "</condition>" );

  append_string( output, "<then>" );
	dump_stmts( output, stmt->true_body);
  append_string( output, "</then>" );

	if (stmt->false_body != NIL)
	{
    append_string( output, "<else>" );
		dump_stmts( output, stmt->false_body);
    append_string( output, "</else>" );
	}

  append_string( output, "</if>" );
}

static void
dump_loop( char ** output, PLpgSQL_stmt_loop *stmt)
{
  append_string( output, "<loop>" );
	dump_stmts( output, stmt->body);
  append_string( output, "</loop>" );
}

static void
dump_while( char ** output, PLpgSQL_stmt_while *stmt)
{
  append_string( output, "<while>" );
  append_string( output, "<condition>" );
	dump_expr( output, stmt->cond);
  append_string( output, "</condition>" );
	dump_stmts( output, stmt->body);
  append_string( output, "</while>" );
}

static void
dump_fori( char ** output, PLpgSQL_stmt_fori *stmt)
{
  append_string( output, "<fori>" );
	printf("FORI %s %s\n", stmt->var->refname, (stmt->reverse) ? "REVERSE" : "NORMAL");

	printf("    lower = ");
	dump_expr( output, stmt->lower);
	printf("\n");
	printf("    upper = ");
	dump_expr( output, stmt->upper);
	printf("\n");
	printf("    step = ");
	dump_expr( output, stmt->step);
	printf("\n");

	dump_stmts( output, stmt->body);

	printf("    ENDFORI\n");
  append_string( output, "</fori>" );
}

static void
dump_fors( char ** output, PLpgSQL_stmt_fors *stmt)
{
  append_string( output, "<fors>" );
	printf("FORS %s ", (stmt->rec != NULL) ? stmt->rec->refname : stmt->row->refname);
	dump_expr( output, stmt->query);
	printf("\n");

	dump_stmts( output, stmt->body);

	printf("    ENDFORS\n");
  append_string( output, "</fors>" );
}

static void
dump_open( char ** output, PLpgSQL_stmt_open *stmt)
{
  append_string( output, "<open/>" );
	printf("OPEN curvar=%d\n", stmt->curvar);

	if (stmt->argquery != NULL)
	{
		printf("  arguments = '");
		dump_expr( output, stmt->argquery);
		printf("'\n");
	}
	if (stmt->query != NULL)
	{
		printf("  query = '");
		dump_expr( output, stmt->query);
		printf("'\n");
	}
	if (stmt->dynquery != NULL)
	{
		printf("  execute = '");
		dump_expr( output, stmt->dynquery);
		printf("'\n");
	}

}

static void
dump_fetch( char ** output, PLpgSQL_stmt_fetch *stmt)
{
  append_string( output, "<fetch>" );

	if (!stmt->is_move)
	{
		printf("FETCH curvar=%d\n", stmt->curvar);
		dump_cursor_direction( output, stmt);

		if (stmt->rec != NULL)
		{
			printf("    target = %d %s\n", stmt->rec->recno, stmt->rec->refname);
		}
		if (stmt->row != NULL)
		{
			printf("    target = %d %s\n", stmt->row->rowno, stmt->row->refname);
		}
	}
	else
	{
		printf("MOVE curvar=%d\n", stmt->curvar);
		dump_cursor_direction( output, stmt);
	}
  append_string( output, "</fetch>" );
}

static void
dump_cursor_direction( char ** output, PLpgSQL_stmt_fetch *stmt)
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
		dump_expr( output, stmt->expr);
		printf("\n");
	}
	else
		printf("%d\n", stmt->how_many);

}

static void
dump_close( char ** output, PLpgSQL_stmt_close *stmt)
{
  append_string( output, "<close/>" );
	printf("CLOSE curvar=%d\n", stmt->curvar);
}

static void
dump_perform( char ** output, PLpgSQL_stmt_perform *stmt)
{
  append_string( output, "<perform>" );
	printf("PERFORM expr = ");
	dump_expr( output, stmt->expr);
	printf("\n");
  append_string( output, "</perform>" );
}

static void
dump_exit( char ** output, PLpgSQL_stmt_exit *stmt)
{
  append_string( output, "<exit>" );
	printf("%s", stmt->is_exit ? "EXIT" : "CONTINUE");
	if (stmt->label != NULL)
		printf(" label='%s'", stmt->label);
	if (stmt->cond != NULL)
	{
		printf(" WHEN ");
		dump_expr( output, stmt->cond);
	}
	printf("\n");
  append_string( output, "</exit>" );
}

static void
dump_return( char ** output, PLpgSQL_stmt_return *stmt)
{
  append_string( output, "<return>" );
	printf("RETURN ");
	if (stmt->retvarno >= 0)
		printf("variable %d", stmt->retvarno);
	else if (stmt->expr != NULL)
		dump_expr( output, stmt->expr);
	else
		printf("NULL");
	printf("\n");
  append_string( output, "</return>" );
}

static void
dump_return_next( char ** output, PLpgSQL_stmt_return_next *stmt)
{
  append_string( output, "<return_next>" );
	printf("RETURN NEXT ");
	if (stmt->retvarno >= 0)
		printf("variable %d", stmt->retvarno);
	else if (stmt->expr != NULL)
		dump_expr( output, stmt->expr);
	else
		printf("NULL");
	printf("\n");
  append_string( output, "</return_next>" );
}

static void
dump_return_query( char ** output, PLpgSQL_stmt_return_query *stmt)
{
  append_string( output, "<return_query>" );
	printf("RETURN QUERY ");
	dump_expr( output, stmt->query);
	printf("\n");
  append_string( output, "</return_query>" );
}

static void
dump_raise( char ** output, PLpgSQL_stmt_raise *stmt)
{
  append_string( output, "<raise>" );
	ListCell   *lc;
	int			i = 0;

	printf("RAISE '%s'\n", stmt->message);
	foreach(lc, stmt->params)
	{
		printf("    parameter %d: ", i++);
		dump_expr( output, (PLpgSQL_expr *) lfirst(lc));
		printf("\n");
	}
  append_string( output, "</raise>" );
}

static void
dump_execsql( char ** output, PLpgSQL_stmt_execsql *stmt)
{
  append_string( output, "<execsql>" );
	printf("EXECSQL ");
	dump_expr( output, stmt->sqlstmt);
	printf("\n");

	if (stmt->rec != NULL)
	{
		printf("    INTO%s target = %d %s\n",
			   stmt->strict ? " STRICT" : "",
			   stmt->rec->recno, stmt->rec->refname);
	}
	if (stmt->row != NULL)
	{
		printf("    INTO%s target = %d %s\n",
			   stmt->strict ? " STRICT" : "",
			   stmt->row->rowno, stmt->row->refname);
	}
  append_string( output, "</execsql>" );
}

static void
dump_dynexecute( char ** output, PLpgSQL_stmt_dynexecute *stmt)
{
  append_string( output, "<execute>" );
	dump_expr( output, stmt->query);

	if (stmt->rec != NULL)
	{
		printf("    INTO%s target = %d %s\n",
			   stmt->strict ? " STRICT" : "",
			   stmt->rec->recno, stmt->rec->refname);
	}
	if (stmt->row != NULL)
	{
		printf("    INTO%s target = %d %s\n",
			   stmt->strict ? " STRICT" : "",
			   stmt->row->rowno, stmt->row->refname);
	}
  append_string( output, "</execute>" );
}

static void
dump_dynfors( char ** output, PLpgSQL_stmt_dynfors *stmt)
{
  append_string( output, "<dynfors>" );
	printf("FORS %s EXECUTE ", (stmt->rec != NULL) ? stmt->rec->refname : stmt->row->refname);
	dump_expr( output, stmt->query);
	printf("\n");

	dump_stmts( output, stmt->body);

	printf("    ENDFORS\n");
  append_string( output, "</dynfors>" );
}

static void
dump_getdiag( char ** output, PLpgSQL_stmt_getdiag *stmt)
{
  append_string( output, "<getdiag>" );
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
  append_string( output, "<getdiag>" );
}

static void
dump_expr( char ** output, PLpgSQL_expr *expr )
{
	int			i;

  append_string( output, "<expression params=\"%d\">", expr->nparams );
  append_string( output, "<query>%s</query>", expr->query );
	if (expr->nparams > 0)
	{
		for (i = 0; i < expr->nparams; i++)
		{
      append_string( output, "<param>$%d=$%d</param>", i + 1, expr->params[i] );
		}
	}
  append_string( output, "</expression>" );
}

