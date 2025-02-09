all:
	gcc -oFast bbHighway.c -o bbHighway
	x86_64-w64-mingw32-gcc -oFast bbHighway.c -o bbHighway.exe

debug:
	gcc  bbHighway.c -o bbHighway
	x86_64-w64-mingw32-gcc bbHighway.c -o bbHighway.exe