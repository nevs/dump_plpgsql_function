
#ifndef _STRING_HELPER_H
#define _STRING_HELPER_H

typedef struct dump_context {
  char ** output;
  int     indent;
} DumpContext;

void string_helper_init();

int 
__attribute__ ((format (printf, 2, 3)))
append_string( char ** buffer, char * fmt, ... );

int xml_tag( DumpContext * context, const char * tagname, ... ) __attribute__((sentinel));
int xml_tag_open( DumpContext * context, const char * tagname, ... ) __attribute__((sentinel));
int xml_tag_close( DumpContext * context, const char * tagname );
int xml_attributes( DumpContext * context, va_list ap );
int xml_content( DumpContext * context, const char * fmt, ... ) __attribute__((format (printf,2,3)));
int xml_textnode( DumpContext * context, const char * tagname, const char * format, ... ) __attribute__((format (printf,3,4)));
DumpContext * new_dump_context();

#endif

