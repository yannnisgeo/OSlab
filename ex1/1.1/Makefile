crtd_objects = main.o zing2.o

all: main zing2
.PHONY: all
	 
main: zing.o main.o
	gcc zing.o main.o -o main

main.o: main.c
	gcc -Wall -c main.c


zing2: zing2.o main.o
	gcc zing2.o main.o -o zing2

zing2.o: zing2.c
	gcc -Wall -c zing2.c

.PHONY: clean

clean: 
	rm -f $(crtd_objects)  main zing2
