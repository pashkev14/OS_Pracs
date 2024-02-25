#include <stdio.h>
#include <unistd.h>
#include <locale.h>
#include <pthread.h>

typedef struct {
	int flag;
	char sym;
} targs;

void* proc1(void* arg) {
	puts("Поток 1 начал работу.\r\n");
	targs* args = (targs*) arg;
	while (args->flag == 0) {
		printf("%c%s", args->sym, "\r\n");
		sleep(1);
	}
	puts("Поток 1 закончил работу.\r\n");
	pthread_exit((void*)10);
}

void* proc2(void* arg) {
	puts("Поток 2 начал работу.\r\n");
	targs* args = (targs*) arg;
	while (args->flag == 0) {
		printf("%c%s", args->sym, "\r\n");
		sleep(1);
	}
	puts("Поток 2 закончил работу.\r\n");
	pthread_exit((void*)20);
}

int main() {
	setlocale(LC_ALL, "RUS");
	puts("Программа начала работу.\r\n");
	int *exitcode1, *exitcode2;
	pthread_t id1, id2;
	pthread_attr_t attr1, attr2;
	size_t size1, size2;
	targs args1, args2;
	args1.flag = 0; args1.sym = '1';
	args2.flag = 0; args2.sym = '2';
	pthread_attr_init(&attr1);
	pthread_attr_init(&attr2);
	pthread_create(&id1, &attr1, proc1, &args1);
	pthread_create(&id2, &attr2, proc2, &args2);
	puts("Программа ждет нажатия клавиши.\r\n");
	getchar();
	puts("Клавиша нажата.\r\n");
	args1.flag = 1;
	args2.flag = 1;
	pthread_join(id1, (void**)&exitcode1);
	pthread_join(id2, (void**)&exitcode2);
	pthread_attr_getstacksize(&attr1, &size1);
	pthread_attr_getstacksize(&attr2, &size2);
	printf("Код завершения потока 1: %p\r\n", exitcode1);
	printf("Размер стека потока 1: %li\r\n", size1);
	printf("Код завершения потока 2: %p\r\n", exitcode2);
	printf("Размер стека потока 2: %li\r\n", size2);
	pthread_attr_destroy(&attr1);
	pthread_attr_destroy(&attr2);
	puts("Программа завершила работу.\r\n");
	return 0;
}
