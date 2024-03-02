#!/bin/bash
./lab4_1.sh
g++ -c lab4_2.c
g++ -o lab4_2 lab4_2.o -lpthread
export PATH="$PWD:$PATH"
./lab4_2 "$@"
