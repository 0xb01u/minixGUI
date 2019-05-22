#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <lib.h>
#include <minix/syslib.h>
#define CLR   "clr"
#define NAME_SIZE 16
#define COMPILE "compila"
#define UP      (char) 0x81
#define DOWN    (char) 0x82
#define RIGHT   (char) 0x83
#define LEFT    (char) 0x84

#define FNAME  ".filelist"
#define LINE_SIZE NAME_SIZE + 2
#define FILES 0
#define OPTIONS 1
#define NOPTIONS 5
#define SOURCE_SIZE 256

#include <unistd.h>
#include <colors.h>

int getStringLine(char **lineptr, FILE *stream);
int countLines(FILE *fp);

char getkey();

int main(void)
{
  FILE *fp;

  char **items;
  /* cat: 0123456789ABCDEF: Is a directory */
  char *checkDirectory;
  int n_items;
  int limit;
  int read = 0;

  const char *options[NOPTIONS];

  int mode = FILES;
  int fSelected = 0;
  int oSelected = 0;
  int *selected = &fSelected;
  char action;

  char *src1;
  char *src2;

  int columns;
  
  int i;

  if ((checkDirectory = (char*) malloc(38 * sizeof (char))) == NULL)
  {
      printf("Error al reservar memoria para la interfaz\n");
      exit(-1);
  }
  
  options[0] = "Parent";
  options[1] = "Console";
  options[2] = "Shut down";
  options[3] = "Reboot";
  options[4] = "Recompile";
  /* "Parent   Console   Shut down   Reboot   Recompile" */

  while (1)
  {
    if ((src1 = (char*) malloc(SOURCE_SIZE * sizeof (char))) == NULL
     || (src2 = (char*) malloc(SOURCE_SIZE * sizeof (char))) == NULL)
    {
      printf("Error al reservar espacio de intercambio\n");
      exit(-2);
    }

    system(CLR);
    system("ls >" FNAME);

    fp = fopen(FNAME, "r");

    n_items = countLines(fp);
    limit = mode == FILES ? n_items : NOPTIONS;

    if ((items = (char**) malloc(n_items  *sizeof (char*))) == NULL)
    {
      printf("Error al reservar memoria para la lista de ficheros\n");
      exit(-3);
    }

    for (i = 0; i < n_items; i++)
    {
      if ((items[i] = (char*) malloc(NAME_SIZE  *sizeof (char))) == NULL)
      {
        printf("Error al reservar memoria para el nombre de los ficheros\n");
        exit(-4);
      }
    }

    i = 0;
    while ((read = getStringLine(&items[i], fp)) != -1)
      if (items[i][0] != '.') i++;

    fclose(fp);

    for (i = 0; i < NOPTIONS; i++)
    {
      if ((mode == OPTIONS) && (oSelected == i))
        printf(MAGENTA"%s   "WHITE, options[i]);
      else
        printf(CYAN"%s   "WHITE, options[i]);
    }
    getcwd(src2, SOURCE_SIZE - 1);
    printf(RED"\nPath: %s\n\n"WHITE, src2);

    for (i = 0; i < n_items; i++)
    {
      if ((mode == FILES) && (fSelected == i))
        printf(GREEN"%s   "WHITE, items[i]);
      else if (items[i][0] != '.')
        printf("%s   ", items[i]);
      if (((i + 1) % NOPTIONS == 0) && (i < n_items - 1))
        printf("\n");
    } 
    printf(YELLOW"\n\nArrows: move   o/a: open/action   r: remove   p: go to parent\nq: options/files   t: console"WHITE"\n\n");

    system("rm "FNAME);


    
    action = getkey();

    if (action == 'q')
    {
      mode = -~mode % 2;
      selected = mode == FILES ? &fSelected : &oSelected;
    }

    else if (action == 'p')
    {
      chdir("..");
      fSelected = 0;
    }

    else if (action == LEFT || action == RIGHT || action == UP || action == DOWN)
    {
      columns = limit < NOPTIONS ? limit : NOPTIONS;

      switch (action)
      {
        case LEFT:
          *selected = (*selected + limit - 1) % limit;
          break;
        case RIGHT:
          *selected = (*selected + 1) % limit;
          break;
        case DOWN:
          *selected = (*selected + columns) >= limit ? (*selected % columns)
            : (*selected + columns);
          break;
        case UP:
          *selected -= columns;
          if (*selected < 0)
          {
            while (*selected + columns < limit)
            {
              *selected += columns;
            }
          }
          break;
        default:
          break;
      }
    }

    else if (mode == FILES)
    {
      if (action == 'o' || action == 'a')
      {
        system(CLR);

        if (chdir(items[fSelected]) == -1)
        {
          strcpy(src1, "cat ");
          system(strcat(src1, items[fSelected]));

          printf("\n");
          getchar();
        }
        else
        {
          fSelected = 0;
        }
      }

      else if (action == 'r')
      {
        printf(WHITE"\nEliminar %s? (Y/n) ", items[fSelected]);
        fflush(stdout);
        if (getkey() == 'Y')
        {
          strcpy(src1, "rm ");
          system(strcat(src1, items[fSelected]));
        }
      }
    }

    if (action == 't')
    {
      mode = OPTIONS;
      action = 'o';
      oSelected = 1;
    }

    if (mode == OPTIONS)
    {
      if (action == 'o' || action == 'a')
      {
        switch (oSelected)
        {
          case 0: chdir("..");
            mode = FILES;
            fSelected = 0;
            break;
          case 1:
            printf("\n"GREEN"MINIX console"WHITE":"BLUE"%s"WHITE"$ ", src2);
            fgets(src1, SOURCE_SIZE - 1, stdin);

            i = 0;
            while (src1[i] != '\n') i++;
            src1[i] = '\0';

            if (src1[0] == 'c' && src1[1] == 'd')
            {
              if (chdir(strcat(src1 + 3, "")) == -1)
              {
                printf("No se encontro el directorio especificado");
              }
              fSelected = 0;
            }
            else system(src1);
            getchar();
            break;
          case 2: system("shutdown -h now");
            break;
          case 3: system("reboot");
            break;
          case 4: system(COMPILE);
            break;
          default: printf("Error al ejecutar opción");
        }
      }
    }

    free(items);
    free(src1);
    free(src2);

  }
  system(CLR);

  return 0;
}

char getkey()
{
  char c;
  message m;
  _taskcall(MM, DELBUF, &m);
  while ((c = m.m1_i1) == 0) _taskcall(MM, GETKEY, &m);
  getchar();
  getchar();
  if ((c & 0x80) && (c & 0x0F))
  {
    getchar();
    getchar();
  }
  return c;
}


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
    return -1;
  }

  return i;
}
