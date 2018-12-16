run:
	gcc rofs.c -o rofs -Wall -ansi -W -std=c99 -g -ggdb -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -lfuse -ltidy
mount:
	./rofs -d mainFolder rofsFolder
unmount:
	fusermount −u ./rofsFolder
