CC=gcc


all: libMagnum.a
	$(CC) -o bb.cgi -D_GNU_SOURCE -D_BSD_SOURCE -Wall -Wvla -std=c99 -O3 bittyblog/main.c bittyblog/db_interface.c bittyblog/bittyblog.c bittyblog/vec.c bittyblog/cgi.c bittyblog/to_json.c -lsqlite3 -lMagnum -L . -Imagnum/src
	$(CC) -o bbadmin.cgi -D_GNU_SOURCE -D_BSD_SOURCE -Wall -Wvla -std=c99 -O3 bittyblog/admin.c bittyblog/db_interface.c bittyblog/bittyblog.c bittyblog/vec.c bittyblog/cgi.c bittyblog/to_json.c -lsqlite3 -lMagnum -L . -Imagnum/src
	rm d_string.o file.o json.o parson.o magnum.o libMagnum.a

bb: libMagnum.a
	$(CC) -o bb.cgi -D_GNU_SOURCE -D_BSD_SOURCE -Wall -Wvla -std=c99 -O3 bittyblog/main.c bittyblog/db_interface.c bittyblog/bittyblog.c bittyblog/vec.c bittyblog/cgi.c bittyblog/to_json.c -lsqlite3 -lMagnum -L . -Imagnum/src
	rm d_string.o file.o json.o parson.o magnum.o libMagnum.a
	mv bb.cgi cgi-bin/bb.cgi

admin: libMagnum.a
	$(CC) -o bbadmin.cgi -D_GNU_SOURCE -D_BSD_SOURCE -Wall -Wvla -std=c99 -O3 bittyblog/admin.c bittyblog/db_interface.c bittyblog/bittyblog.c bittyblog/vec.c bittyblog/cgi.c bittyblog/to_json.c -lsqlite3 -lMagnum -L . -Imagnum/src
	mv bbadmin.cgi cgi-bin/bbadmin.cgi

libMagnum.a: d_string.o file.o json.o parson.o magnum.o
	ar rcs libMagnum.a d_string.o file.o json.o parson.o magnum.o

d_string.o:
	gcc -c -o d_string.o magnum/src/d_string.c

file.o:
	gcc -c -o file.o magnum/src/file.c

json.o:
	gcc -c -o json.o magnum/src/json.c

parson.o:
	gcc -c -o parson.o magnum/src/parson.c

magnum.o:
	gcc -c -o magnum.o magnum/src/magnum.c

.PHONY : clean
clean:
	rm d_string.o file.o json.o parson.o magnum.o libMagnum.a
