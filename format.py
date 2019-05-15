def include(dest, file):
	dest.write("\n")

	f = open(file, 'r')
	for line in f:
		dest.write(line)

def __main__():
	exclusive = 0
	begin = 0

	f = open("GUI_dev.c", 'r')
	d = open("GUI.c", 'w')
	for line in f:
		if line[:10] == "#include \"":
			if line[10:-2] == "include/colors.h":
				d.write("#include <colors.h>\n")
			else:
				include(d, line[10:-2])
		elif line == "#if defined(__minix)\n":
			exclusive = 1
			begin = 2
		elif exclusive == 1 and line == "#else\n":
			exclusive = 2
		elif exclusive != 2:
			d.write(line[begin:])
		else:
			if line == "#endif\n":
				exclusive = 0
				begin = 0
	f.close()
	d.write("\n\n")

	f = open("ftools.c", 'r')
	tools = False

	for line in f:
		if line[0:3] == "int":
			tools = True
		if tools:
			d.write(line)
	f.close()

	d.close()

if __name__ == "__main__":
	__main__()