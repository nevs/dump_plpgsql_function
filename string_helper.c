
#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>

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

void string_helper_init()
{

}

