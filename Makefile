CC=gcc
FCGI=y

all: libMagnum.a
ifeq ($(FCGI),y)
	$(CC) -o bb.cgi -D_DEFAULT_SOURCE -D_FCGI -Wall -Wvla -std=c99 -O3 bittyblog/main.c bittyblog/db_interface.c bittyblog/bittyblog.c bittyblog/vec.c bittyblog/cgi.c bittyblog/to_json.c -lsqlite3 -lfcgi -lMagnum -L . -Imagnum/src
	$(CC) -o bbadmin.cgi -D_DEFAULT_SOURCE -D_FCGI -Wall -Wvla -std=c99 -O3 bittyblog/admin.c bittyblog/db_interface.c bittyblog/bittyblog.c bittyblog/vec.c bittyblog/cgi.c bittyblog/to_json.c -lsqlite3 -lfcgi -lMagnum -L . -Imagnum/src
else
	$(CC) -o bb.cgi -D_DEFAULT_SOURCE -Wall -Wvla -std=c99 -O3 bittyblog/main.c bittyblog/db_interface.c bittyblog/bittyblog.c bittyblog/vec.c bittyblog/cgi.c bittyblog/to_json.c -lsqlite3 -lMagnum -L . -Imagnum/src
	$(CC) -o bbadmin.cgi -D_DEFAULT_SOURCE -Wall -Wvla -std=c99 -O3 bittyblog/admin.c bittyblog/db_interface.c bittyblog/bittyblog.c bittyblog/vec.c bittyblog/cgi.c bittyblog/to_json.c -lsqlite3 -lMagnum -L . -Imagnum/src
endif
	rm d_string.o file.o json.o parson.o magnum.o libMagnum.a

bb: libMagnum.a
ifeq ($(FCGI),y)
	$(CC) -o bb.cgi -D_DEFAULT_SOURCE -D_FCGI -Wall -Wvla -std=c99 -O3 bittyblog/main.c bittyblog/db_interface.c bittyblog/bittyblog.c bittyblog/vec.c bittyblog/cgi.c bittyblog/to_json.c -lsqlite3 -lfcgi -lMagnum -L . -Imagnum/src
else
	$(CC) -o bb.cgi -D_DEFAULT_SOURCE -Wall -Wvla -std=c99 -O3 bittyblog/main.c bittyblog/db_interface.c bittyblog/bittyblog.c bittyblog/vec.c bittyblog/cgi.c bittyblog/to_json.c -lsqlite3 -lMagnum -L . -Imagnum/src
endif
	rm d_string.o file.o json.o parson.o magnum.o libMagnum.a
	mv bb.cgi cgi-bin/bb.cgi

admin: libMagnum.a
ifeq ($(FCGI),y)
	$(CC) -o bbadmin.cgi -D_DEFAULT_SOURCE -D_FCGI -Wall -Wvla -std=c99 -O3 bittyblog/admin.c bittyblog/db_interface.c bittyblog/bittyblog.c bittyblog/vec.c bittyblog/cgi.c bittyblog/to_json.c -lsqlite3 -lfcgi -lMagnum -L . -Imagnum/src
else
	$(CC) -o bbadmin.cgi -D_DEFAULT_SOURCE -Wall -Wvla -std=c99 -O3 bittyblog/admin.c bittyblog/db_interface.c bittyblog/bittyblog.c bittyblog/vec.c bittyblog/cgi.c bittyblog/to_json.c -lsqlite3 -lMagnum -L . -Imagnum/src
endif
	mv bbadmin.cgi cgi-bin/bbadmin.cgi

libMagnum.a: d_string.o file.o json.o parson.o magnum.o
	ar rcs libMagnum.a d_string.o file.o json.o parson.o magnum.o

d_string.o:
	gcc -c -o d_string.o -O3 magnum/src/d_string.c

file.o:
	gcc -c -o file.o -O3 magnum/src/file.c

json.o:
	gcc -c -o json.o -O3 magnum/src/json.c

parson.o:
	gcc -c -o parson.o -O3 magnum/src/parson.c

magnum.o:
	gcc -c -o magnum.o -O3 magnum/src/magnum.c

.PHONY : clean
clean:
	rm d_string.o file.o json.o parson.o magnum.o libMagnum.a
