#if !defined(__minix)
  #include "ubdev.h"
#endif

_PROTOTYPE(int getStringLine, (char **lineptr, FILE *stream));
_PROTOTYPE(int countLines, (FILE *fp));