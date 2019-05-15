#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/colors.h"
#include "include/ftools.h"

#define FNAME  ".filelist"
#define DNAME  ".isdirectory"
#define LINE_SIZE NAME_SIZE + 2
#define FILES 0
#define OPTIONS 1
#define NOPTIONS 5
#define UP      'A'
#define DOWN    'B'
#define RIGHT   'C'
#define LEFT    'D'
#define SOURCE_SIZE 256

#if defined(__minix)
  #include <sys/types.h>
  #define CLR   "clr"
  #define NAME_SIZE 16
  #define COMPILE "compila"
#else
  #include "include/ubdev.h"
#endif
#include <unistd.h>

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
    printf(BLUE"\nPath: %s\n\n"WHITE, src2);

    for (i = 0; i < n_items; i++)
    {
      if ((mode == FILES) && (fSelected == i))
        printf(GREEN"%s   "WHITE, items[i]);
      else if (items[i][0] != '.')
        printf("%s   ", items[i]);
      if ((i + 1) % NOPTIONS == 0)
        printf("\n");
    } 
    printf(YELLOW"\nArrows: move   o/a: open/action   p: go to parent   q: options/files\n"WHITE);

    system("rm "FNAME);
    
    action = getchar();

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

    else if (action == '\033')
    {
      getchar();
      columns = limit < NOPTIONS ? limit : NOPTIONS;

      switch (getchar())
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

        strcpy(src1, "cat ");
        strcpy(src2, " 2> "DNAME);

        system(strcat(strcat(src1, items[fSelected]), src2));

        fp = fopen(DNAME, "r");
        if (getStringLine((char **) &checkDirectory, fp) != -1)
        {
          strcpy(src1, "./");
          chdir(strcat(src1, items[fSelected]));
          fSelected = 0;
        }
        else
      {
          printf("\n\n");
          getchar();
      }

        system("rm "DNAME);
        fclose(fp);
      }

      else if (action == 'e')
      {
        free(items);
        break;
      }
    }

    else if (mode == OPTIONS)
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
                printf("No se encontró el directorio especificado");
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

    while (getchar() != '\n');
  }
  system(CLR);

  return 0;
}