#!/bin/sh

gcc -O2 -Wall -Wextra  -c main.c -o obj/main.o
gcc -O2 -Wall -Wextra  -c test/test.c -o obj/test/test.o
gcc -O2 -Wall -Wextra  -c src/a.c -o obj/src/a.o
g++ -O2 -Wall -Wextra  -c src/main2.cpp -o obj/src/main2.o
gcc -O2 -Wall -Wextra  -c src/test/test.c -o obj/src/test/test.o
gcc -O2 -Wall -Wextra  -c src/a1.c -o obj/src/a1.o

g++ -s   obj/main.o obj/test/test.o obj/src/a.o obj/src/main2.o obj/src/test/test.o obj/src/a1.o  -o rmake 


