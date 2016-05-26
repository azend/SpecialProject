

ast2016.exe: main.obj
	cl main.obj /Fast2016.exe

.c.obj:
	cl $*.c /c

