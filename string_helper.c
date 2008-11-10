
#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <printf.h>

#include <postgres.h>

#include "string_helper.h"

/** helper function to append c string to existing c string */
int 
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

/*
 * FIXME: This is not safe for all encodings.
 *
 */

static int print_xml(FILE *stream, const struct printf_info *info, const void *const *args)
{
  const char * xml;
  int written = 0;

  xml = *((const char **) args[0]);
  for( ; *xml; xml++ ){
    int len;
    switch( *xml ) {
      case '<':
        len = fprintf( stream, "%s", "&lt;" );
        break;
      case '>':
        len = fprintf( stream, "%s", "&gt;" );
        break;
      case '&':
        len = fprintf( stream, "%s", "&amp;" );
        break;
      default:
        len = fprintf( stream, "%c", *xml );
        break;
    }
    if ( len < 0 ) 
      return len;
    else
      written += len;
  }

  return written;
}

static int print_xml_arginfo (const struct printf_info *info, size_t n, int *argtypes)
{
  /* We always take exactly one argument and this is a pointer to the structure.. */
  if (n > 0)
    argtypes[0] = PA_POINTER;

  return 1;
}

void string_helper_init()
{
  register_printf_function('M', print_xml, print_xml_arginfo);
}

int xml_tag( DumpContext * context, const char * tagname, ... )
{
  return append_string( context->output, "<%M/>", tagname );
}

int xml_tag_open( DumpContext * context, const char * tagname, ... )
{
  return append_string( context->output, "<%M>", tagname );
}

int xml_tag_close( DumpContext * context, const char * tagname )
{
  return append_string( context->output, "</%M>", tagname );
}

int xml_content( DumpContext * context, const char * content )
{
  return append_string( context->output, "%M", content );
}

