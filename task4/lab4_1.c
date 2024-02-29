#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "RUS");
	puts("Программа 1 начала свою работу.\r\n");
	printf("ID процесса программы 1: %d\r\n", getpid());
	printf("ID родительского процесса программы 1: %d\r\n", getppid());
	puts("Аргументы, переданные в программу 1:\r\n");
	for (int i = 0; i < argc; ++i) {
		printf("Аргумент %d: %s\r\n", i + 1, argv[i]);
		sleep(1);
	}
	puts("Программа 1 закончила свою работу.\r\n");
	exit(2);
}
