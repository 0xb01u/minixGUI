#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__minix)
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
#else
  #include "include/ubdev.h"
#endif

#define FNAME  ".filelist"
#define LINE_SIZE NAME_SIZE + 2
#define FILES 0
#define OPTIONS 1
#define NOPTIONS 5
#define SOURCE_SIZE 256

#include <unistd.h>
#include "include/colors.h"
#include "include/ftools.h"

char getkey();

int main(void)
{
  /*
   * Fichero que guarda los nombres de los ficheros del directorio.
   */
  FILE *fp;

  char **items; /* Array de nombres de los ficheros */
  int n_items;  /* Numero de ficheros del directorio */
  int limit;    /* Numero de ficheros por fila a imprimir */
  int columns;  /* Número de columnas de la interfaz */

  const char *options[NOPTIONS];  /* Array de nombres de las opciones */

  int mode = FILES;   /* Modo de ejecucion de la GUI */
  int fSelected = 0;  /* Fichero seleccionado actualmente */
  int oSelected = 0;  /* Opcion seleccionada actualmente */
  int *selected = &fSelected; /* Puntero a la variable de selección
                                correspondiente al modo actual */
  char action;  /* Tecla que ha pulsado el usuario */

  /* Buffers para almacenar Strings y poder concatenarlas */
  char *src1;
  char *src2;
  
  int i;  /* Variable para iterar en bucles */
  
  /* Opciones a imprimir en la primera fila: */
  options[0] = "Parent";
  options[1] = "Console";
  options[2] = "Shut down";
  options[3] = "Reboot";
  options[4] = "Recompile";
  /* "Parent   Console   Shut down   Reboot   Recompile" */

  /* Bucle principal: */
  while (1)
  {
    /* Alocación de la memoria de los buffers de Strings.
       (El tamaño es fijo, pero no se puede usar arrays porque
       da un error en otro punto del programa.) */
    if ((src1 = (char*) malloc(SOURCE_SIZE * sizeof (char))) == NULL
     || (src2 = (char*) malloc(SOURCE_SIZE * sizeof (char))) == NULL)
    {
      printf("Error al reservar espacio de intercambio\n");
      exit(-2);
    }

    system(CLR);
    system("ls >" FNAME); /* Creación del fichero de nombres */

    fp = fopen(FNAME, "r");

    n_items = countLines(fp);
    limit = mode == FILES ? n_items : NOPTIONS;

    /* Alocacion de la memoria del buffer de nombres */
    if ((items = (char**) malloc(n_items  *sizeof (char*))) == NULL)
    {
      printf("Error al reservar memoria para la lista de ficheros\n");
      exit(-3);
    }

    for (i = 0; i < n_items; i++)
    {
      /* Alocacion de memoria para cada nombre del buffer */
      if ((items[i] = (char*) malloc(NAME_SIZE  *sizeof (char))) == NULL)
      {
        printf("Error al reservar memoria para el nombre de los ficheros\n");
        exit(-4);
      }
    }

    /* Almacenamiento de cada nombre en el buffer
       (los ficheros que empiezan por '.' se ocultan). */
    i = 0;
    while (getStringLine(&items[i], fp) != -1)
      if (items[i][0] != '.') i++;

    fclose(fp);

    /* Impresión de opciones, path, ficheros del directorio e indicaciones: */
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

    system("rm "FNAME); /* Eliminamos el fichero de nombres */


    
    action = getkey();  /* Capturamos la tecla del usuario */

    if (action == 'q')  /* Cambiar de modo */
    {
      mode = -~mode % 2;
      selected = mode == FILES ? &fSelected : &oSelected;
    }

    else if (action == 'p') /* Ir al directorio padre */
    {
      chdir("..");
      fSelected = 0;
    }

    else if (action == LEFT || action == RIGHT || action == UP || action == DOWN)
    /* Nos movemos entre la lista de ficheros u opciones */
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
      if (action == 'o' || action == 'a') /* Abrir un fichero o directorio */
      {
        system(CLR);

        if (chdir(items[fSelected]) == -1)
        {
          /* El fichero no es un directorio */
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

      else if (action == 'r') /* Eliminar el fichero */
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

    if (action == 't')  /* Abrir consola */
    {
      mode = OPTIONS;
      action = 'o';
      oSelected = 1;
    }

    if (mode == OPTIONS)
    {
      if (action == 'o' || action == 'a') /* Ejecutar opcion  */
      {
        switch (oSelected)
        {
          case 0: /* Ir al directorio padre */
            chdir("..");
            mode = FILES;
            fSelected = 0;
            break;
          case 1: /* Abrir consola */
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
          case 2: system("shutdown -h now");  /* Apagar el sistema */
            break;
          case 3: system("reboot"); /* Reiniciar el sistema */
            break;
          case 4: system(COMPILE);  /* Recompilar el núcleo */
            break;
          default: printf("Error al ejecutar opción");
        }
      }
    }

    /* Liberamos memoria que vamos a reutilizar
       (si no, se llena la memoria del sistema) */
    free(items);
    free(src1);
    free(src2);

  }
  /* Borrar pantalla */
  system(CLR);

  return 0;
}

#if defined(__minix)
  char getkey()
  {
    char c;
    message m;
  
    /* Nos aseguramos de que el valor de tty_intail es 0: */
    _taskcall(MM, DELBUF, &m);
    /* Devolvemos el valor de tty_intail hasta que no sea 0 (se ha pulsado una tecla) */
    while ((c = m.m1_i1) == 0) _taskcall(MM, GETKEY, &m);
    /* Descartamos que la terminal use la tecla pulsada y el '\n' introducido */ 
    getchar();
    getchar();
  
    if ((c & 0x80) && (c & 0x0F)) /* Se ha pulsado una flecha */
    {
      /* Descartamos los caracteres extra introducidos */
      getchar();
      getchar();
    }
  
    return c;
  }
#else
  char getkey()
  {
    char a;
    a = getchar();
    getchar();
    if (a == '\033')
    {
      a = getchar();
      getchar();
    }

    return a;
  }
#endif
