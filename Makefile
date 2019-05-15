GUI : GUI.o ftools.o
	cc -o GUI GUI.o ftools.o
	rm GUI.o ftools.o

GUI.o : GUI.c include/colors.h include/ftools.h include/ubdev.h
	cc -c GUI.c
ftools.o : ftools.c
	cc -c ftools.c