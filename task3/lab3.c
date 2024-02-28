#include <stdio.h>
#include <unistd.h>
#include <locale.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/resource.h>

int flag1 = 0, flag2 = 0;
int pipefd[2];
struct rusage usage;

void* proc1(void* args) {
	puts("Поток записи начал работу.\r\n");
	char buf[256];
	while (flag1 == 0) {
		memset(buf, 0, 256);
		getrusage(RUSAGE_SELF, &usage);
		size_t count = (size_t)(sprintf(buf, "%li", usage.ru_maxrss));
		ssize_t rv = write(pipefd[1], buf, count);
		if (rv == -1) {
			printf("Поток записи остановился с ошибкой %s\r\n\r\n", strerror(rv));
		}
		else if (rv >= 0) {
			printf("Поток записи записал в канал следующее сообщение: %s\r\nЧисло фактически записанных в канал байт: %li\r\n\r\n", buf, rv);
		}
		sleep(1);
	}
	puts("Поток записи закончил работу.\r\n");
	pthread_exit(NULL);
}

void* proc2(void* args) {
	puts("Поток чтения начал работу.\r\n");
	char buf[256];
	while (flag2 == 0) {
		memset(buf, 0, 256);
		ssize_t rv = read(pipefd[0], buf, (size_t)256);
		if (rv == -1) {
			printf("Поток чтения остановился с ошибкой %s\r\n\r\n", strerror(rv));
		}
		else if (rv == 0) {
			printf("Поток чтения достиг конца файла\r\n\r\n");
		}
		else if (rv > 0) {
			printf("Поток чтения успешно считал с канала в буфер сообщение: %s\r\n\r\n", buf);
		}
		sleep(1);
	}
	puts("Поток чтения закончил работу.\r\n");
	pthread_exit(NULL);
}

int main() {
	setlocale(LC_ALL, "RUS");
	puts("Программа начала работу.\r\n");
	pthread_t id1, id2;
	int rv = pipe(pipefd);
	fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
	fcntl(pipefd[1], F_SETFL, O_NONBLOCK);
	puts("Программа ждет нажатия клавиши.\r\n");
	pthread_create(&id1, NULL, proc1, NULL);
	pthread_create(&id2, NULL, proc2, NULL);
	getchar();
	puts("Клавиша нажата.\r\n");
	flag1 = 1;
	flag2 = 1;
	pthread_join(id1, NULL);
	pthread_join(id2, NULL);
	close(pipefd[0]);
	close(pipefd[1]);
	puts("Программа завершила работу.\r\n");
	return 0;
}
