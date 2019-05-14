GUI : GUI.o filetools.o
	gcc -o GUI GUI.o filetools.o
	rm GUI.o filetools.o

GUI.o : GUI.c include/colors.h include/filetools.h include/ubuntudev.h
	gcc -c GUI.c
filetools.o : filetools.c
	gcc -c filetools.c