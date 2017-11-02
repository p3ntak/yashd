# The program we want to build, what it depends on
# and how to build it

all: yashd yash

yashd : yashd.o
	gcc-6 -o yashd yashd.o -lpthread

yash : yash.o
	gcc-6 -o yash yash.o

# yashd.o depends on yashd.c and it is built
# by running the command gcc -c yashd.c

yashd.o : yashd.c daemon.h helpers.h my_semaphore.h threads.h yash.c
	gcc-6 -c yashd.c -lpthread

yash.o : yash.c
	gcc-6 -c yash.c

# What to do if make is run as:
#   make clean
# remote all object and executables
clean:
	rm *.o yashd yash
