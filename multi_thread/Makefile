CFILES=dlist.c  locker_pthread.c
all:
	gcc -g -fPIC -shared -lpthread $(CFILES) -o libdlist.so
	gcc -g -DDLIST_TEST $(CFILES) -o dlist_test -std=c99 -lpthread

clean:
	rm -f *test *.exe *.so
