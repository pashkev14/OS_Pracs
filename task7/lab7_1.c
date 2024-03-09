#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <locale.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/resource.h>

typedef struct {
	long mtype;
	char mtext[256];
} msg_t;

pthread_t thread;
int msgid;
int len;
int rv;
int flag = 0;
msg_t message;
struct rusage usage;

int generate_message(char* buf) {
	if (getrusage(RUSAGE_SELF, &usage) == -1) {
		return -1;
	}
	memset(buf, 0, 256);
	int size = sprintf(buf, "Максимальный объем резидентной памяти: %li", usage.ru_maxrss);
	return size;
}

void* proc(void* args) {
	puts("Программа 1: поток передачи сообщений начал свою работу.\r\n");
	while (flag == 0) {
		len = generate_message(message.mtext);
		if (len < 0) {
			perror("Программа 1: ошибка генерации сообщения");
		}
		rv = msgsnd(msgid, &message, len, IPC_NOWAIT);
		if (rv == -1) {
			printf("Программа 1: ошибка передачи сообщения: %s\r\n", strerror(errno));
		}
		else if (rv == 0) {
			printf("Сгенерировано и записано сообщение: %s\r\n", message.mtext);
		}
		sleep(1);
	}
	puts("Программа 1: поток передачи сообщений закончил свою работу.\r\n");
	pthread_exit(NULL);
}

int main() {
	setlocale(LC_ALL, "RUS");
	puts("Программа 1 начала свою работу.\r\n");
	key_t key = ftok("lab7", 'A');
	message.mtype = 1;
	msgid = msgget(key, 0);
	if (msgid < 0) {
		msgid = msgget(key, IPC_CREAT | 0644);
	}
	pthread_create(&thread, NULL, proc, NULL);
	puts("Программа ждет нажатия на клавишу.\r\n");
	getchar();
	puts("Программа 1: клавиша нажата.\r\n");
	flag = 1;
	pthread_join(thread, NULL);
	msgctl(msgid, IPC_RMID, NULL);
	puts("Программа 1 закончила свою работу.\r\n");
	return 0;
}
