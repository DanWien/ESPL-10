
all: loader

loader: task2.o startup.o start.o
	ld -o loader task2.o startup.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib/ld-linux.so.2

task2.o: task2.c 
	gcc -m32 -c task2.c -o task2.o

start: start.o
	 gcc -g -m32 -Wall -o start start.o

 # Depends on the source and header files 

start.o: start.s
	nasm -g -f elf -w+all -o start.o start.s  

startup: startup.o
	 gcc -g -m32 -Wall -o startup startup.o

 # Depends on the source and header files 

startup.o: startup.s
	nasm -g -f elf -w+all -o startup.o startup.s  

.PHONY: clean

clean:
	rm -f *.o loader