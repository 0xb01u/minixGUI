GUI.c : GUI_dev.c include/ftools.h ftools.c format.py
	python format.py

GUI_ubuntu : GUI_dev.o ftools.o
	cc -o GUI_ubuntu GUI_dev.o ftools.o
	rm GUI_dev.o ftools.o

GUI_dev.o : GUI.c include/colors.h include/ftools.h include/ubdev.h
	cc -c GUI_dev.c
ftools.o : ftools.c
	cc -c ftools.c

dev ::