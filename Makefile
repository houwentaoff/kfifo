default:all

all:main

main:main.o kfifo.o
	gcc  $^ -o $@ 

%.o:%.c
	gcc -c $^ -o $@  -g

.PHONY:
clean:
	rm *.o main
