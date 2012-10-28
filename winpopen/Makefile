CC=i686-w64-mingw32-gcc

winpopen.dll: winpopen.c
	$(CC) -shared -o winpopen.dll winpopen.c -llua52 -L .
	strip --strip-unneeded winpopen.dll
