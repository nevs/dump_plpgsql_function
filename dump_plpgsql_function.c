
#include <postgres.h>
#include <fmgr.h>
#include <plpgsql.h>
#include <stdio.h>

#define BUFSIZE 4096

PG_MODULE_MAGIC;

/*
 * CREATE OR REPLACE FUNCTION dump_plpgsql_function(text) returns text AS '/home/sven/diplom/dump_plpgsql_function/dump_plpgsql_function.so', 'dump_plpgsql_function' LANGUAGE C STRICT;
 *
 */

static char * dumptree(PLpgSQL_function *func);

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


static char * dumptree(PLpgSQL_function *func)
{
	int			i;
	PLpgSQL_datum *d;

	printf("\nExecution tree of successfully compiled PL/pgSQL function %s:\n",
		   func->fn_name);

	printf("\nFunction's data area:\n");
	for (i = 0; i < func->ndatums; i++)
	{
		d = func->datums[i];

		printf("    entry %d: ", i);
		switch (d->dtype)
		{
			case PLPGSQL_DTYPE_VAR:
				{
					PLpgSQL_var *var = (PLpgSQL_var *) d;

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
				printf("??? unknown data type %d\n", d->dtype);
		}
	}
	printf("\nFunction's statements:\n");

	printf("%3d:", func->action->lineno);
	dump_block(func->action);
	printf("\nEnd of execution tree of function %s\n\n", func->fn_name);
	fflush(stdout);
}
