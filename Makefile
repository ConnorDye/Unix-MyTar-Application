CC = gcc
CFLAGS = -Wall -g -pedantic -std=gnu99 
LD = gcc
LDFLAGS =

l: listarch.o
	$(LD) $(LDFLAGS) -o l listarch.o

c: createArchive.o convertOctal.o
	$(LD) $(LDFLAGS) -o c createArchive.o convertOctal.o

x: extractArchive.o convertOctal.o
	$(LD) $(LDFLAGS) -o x extractArchive.o convertOctal.o

mytar: mytar.o createArchive.o convertOctal.o extractArchive.o listarch.o
	$(LD) $(LDFLAGS) -o mytar mytar.o createArchive.o convertOctal.o extractArchive.o listarch.o

mytar.o: mytar.c
	$(CC) $(CFLAGS) -c -o mytar.o mytar.c

convertOctal.o: convertOctal.c
	$(CC) $(CFLAGS) -c -o convertOctal.o convertOctal.c

listarch.o: listarch.c
	$(CC) $(CFLAGS) -c -o listarch.o listarch.c

extractArchive.o: extractArchive.c
	$(CC) $(CFLAGS) -c -o extractArchive.o extractArchive.c

createArchive.o: createArchive.c
	$(CC) $(CFLAGS) -c -o createArchive.o createArchive.c

test:
	./mytar

git:
	git add extractArchive.c convertOctal.c mytar.c createArchive.c create.h listarch.c README Makefile
	git commit -m "$m"
	git push -u origin master

clean:
	rm *.o 

scp:
	scp mytar.c convertOctal.c createArchive.c listarch.c  create.h extractArchive.c README Makefile chdye@unix2.csc.calpoly.edu:./asgn4

handin:
	handin pn-cs357 asgn4 README Makefile mytar.c extractArchive.c convertOctal.c mytar.c createArchive.c create.h listarch.c

valgrind:
	valgrind --leak-check=yes --track-origins=yes ./mytar

gdb:
	gdb --args ./mytar cvs test ./testdir1

gdb2:
	gdb --args ./mytar xvs ./test 
    
