
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

  va_start( ap, fmt );
  int len = vasprintf(&tmp, fmt, ap);
  va_end( ap );

  if ( len == -1 ) {
    /* error */
    return -1;
  } else {
    size_t offset = 0;
    if ( !*buffer ) {
      *buffer = (char *) palloc( len + 1 );
    } else {
      offset = strlen( *buffer );
      *buffer = (char *) repalloc( *buffer, offset + len + 1 );
    }
    if (!*buffer) {
      free( tmp );
      return -1;
    } else {
      memcpy( *buffer + offset, tmp, len + 1 );
      free( tmp );
    }
    return len;
  }
}

/*
 * FIXME: This is not safe for all encodings.
 * FIXME: integer overflow
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
      case '"':
        if ( info->spec == 'N' ) {
          len = fprintf( stream, "%s", "&quot;" );
        } else {
          len = fprintf( stream, "%c", *xml );
        }
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
  register_printf_function('N', print_xml, print_xml_arginfo);
}

static void xml_indent( DumpContext * context )
{
  int i;
  for( i=0; i<context->indent; i++ )
    append_string( context->output, "  " );
}

int xml_tag( DumpContext * context, const char * tagname, ... )
{
  va_list ap;
  int written = 0, len;

  xml_indent( context );

  len = append_string( context->output, "<%N", tagname );
  if ( len < 0 ) return len;
  written += len;

  va_start( ap, tagname );
  len = xml_attributes( context, ap );
  va_end( ap );
  if ( len < 0 ) return len;
  written += len;

  len = append_string( context->output, "/>\n" );
  if ( len < 0 ) return len;
  written += len;

  return written;
}

int xml_tag_open( DumpContext * context, const char * tagname, ... )
{
  va_list ap;
  int written = 0, len, i;

  xml_indent( context );

  len = append_string( context->output, "<%N", tagname );
  if ( len < 0 ) return len;
  written += len;

  va_start( ap, tagname );
  len = xml_attributes( context, ap );
  va_end( ap );
  if ( len < 0 ) return len;
  written += len;

  len = append_string( context->output, ">\n" );
  if ( len < 0 ) return len;
  written += len;

  context->indent++;

  return written;
}

int xml_attributes( DumpContext * context, va_list ap )
{
  int written = 0, len;
  char *attribute, *value;
  while( 1 ) {
    attribute = va_arg( ap, char * );
    if ( attribute ) {
      value = va_arg( ap, char * );
      if ( value ) {
        len = append_string( context->output, " %N=\"%N\"", attribute, value );
        if ( len < 0 ) return len;
        written += len;
      } else {
        break;
      }
    } else {
      break;
    }
  }
  return written;
}

int xml_tag_close( DumpContext * context, const char * tagname )
{
  context->indent--;
  xml_indent( context );
  return append_string( context->output, "</%N>\n", tagname );
}

int xml_content( DumpContext * context, const char * fmt, ... )
{
  va_list ap;
  char * tmp;

  va_start( ap, fmt );
  int len = vasprintf(&tmp, fmt, ap);
  va_end( ap );

  xml_indent( context );
  return append_string( context->output, "%M\n", tmp );
}

