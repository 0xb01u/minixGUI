#include <stdio.h>
#include <string.h>
#include "filetools.h"

int getStringLine(char **lineptr, FILE *stream)
  {
    static char line[LINE_SIZE];
    unsigned int len;
    char c;
    int i = 0;

    if (lineptr == NULL)
    {
      return -1;
    }

    if (ferror(stream))
      return -1;

    if (feof(stream))
      return -1;
      
    if (fgets(line, LINE_SIZE, stream) == NULL)
      return -1;

    while (line[i++] != '\n');
    line[--i] = '\0';

    if ((c = fgetc(stream)) != EOF)
    	ungetc((int) c, stream);

    len = strlen(line);

    strcpy(*lineptr, line);
    return(len);
  }

int countLines(FILE *fp)
{
  int i = 0;
  char *line;
  int read = 0;

  while ((read = getStringLine(&line, fp)) != -1)
  {
    i++;
    if (line[0] == '.') i--;
  }

  if (fseek(fp, 0L, SEEK_SET) != 0)
  {
    printf("Error al cargar el número de ficheros\n");
    exit(-1);
  }

  return i;
}
