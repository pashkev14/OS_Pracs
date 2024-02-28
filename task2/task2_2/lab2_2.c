#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <string.h>

int flag1 = 0, flag2 = 0;
int i, j;
pthread_mutex_t mutex;

void sig_handler(int signo) {
	printf("\r\nCaught SIGINT, code %d\r\n", signo);
	pthread_mutex_destroy(&mutex);
	exit(0);
}

void* proc1(void* args) {
	puts("Поток 1 начал работу.\r\n");
	while (flag1 == 0) {
		pthread_mutex_lock(&mutex);
		for (i = 0; i < 10; ++i) {
			putchar('1');
			fflush(stdout);
			sleep(1);
		}
		pthread_mutex_unlock(&mutex);
		sleep(1);
	}
	puts("\r\nПоток 1 закончил работу.\r\n");
	pthread_exit(NULL);
}

void* proc2(void* args) {
	puts("\r\nПоток 2 начал работу.\r\n");
	while (flag2 == 0) {
		pthread_mutex_lock(&mutex);
		for (j = 0; j < 10; ++j) {
			putchar('2');
			fflush(stdout);
			sleep(1);
		}
		pthread_mutex_unlock(&mutex);
		sleep(1);
	}
	puts("\r\nПоток 2 закончил работу.\r\n");
	pthread_exit(NULL);
}

int main() {
	setlocale(LC_ALL, "RUS");
	signal(SIGINT, sig_handler);
	puts("Программа начала работу.\r\n");
	pthread_t id1, id2;
	pthread_mutex_init(&mutex, NULL);
	// pthread_mutex_lock(&mutex); // для сравнения программ 2 и 3 убрать комментарий здесь
	puts("Программа ждет нажатия клавиши.\r\n");
	pthread_create(&id1, NULL, proc1, NULL);
	pthread_create(&id2, NULL, proc2, NULL);
	getchar();
	puts("Клавиша нажата.\r\n");
	flag1 = 1;
	flag2 = 1;
	pthread_join(id1, NULL);
	pthread_join(id2, NULL);
	pthread_mutex_destroy(&mutex);
	puts("Программа завершила работу.\r\n");
	return 0;
}
