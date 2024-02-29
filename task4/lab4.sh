#!/bin/bash
g++ -c lab4_1.c
g++ -c lab4_2.c
g++ -o lab4_1 lab4_1.o -lpthread
g++ -o lab4_2 lab4_2.o -lpthread
PATH=$PATH:$(dirname $0)
