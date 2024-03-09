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
int rv;
int flag = 0;
msg_t message;

void* proc(void* args) {
	puts("Программа 2: поток приема сообщений начал свою работу.\r\n");
	while (flag == 0) {
		memset(message.mtext, 0, 256);
		rv = msgrcv(msgid, &message, 256, message.mtype, IPC_NOWAIT);
		if (rv == -1) {
			printf("Программа 2: ошибка приема сообщения: %s\r\n", strerror(errno));
		}
		else {
			printf("Считано сообщение: %s\r\n", message.mtext);
		}
		sleep(1);
	}
	puts("Программа 2: поток приема сообщений закончил свою работу.\r\n");
	pthread_exit(NULL);
}

int main() {
	setlocale(LC_ALL, "RUS");
	puts("Программа 2 начала свою работу.\r\n");
	key_t key = ftok("lab7", 'A');
	message.mtype = 1;
	msgid = msgget(key, 0);
	if (msgid < 0) {
		msgid = msgget(key, IPC_CREAT | 0644);
	}
	pthread_create(&thread, NULL, proc, NULL);
	puts("Программа ждет нажатия на клавишу.\r\n");
	getchar();
	puts("Программа 2: клавиша нажата.\r\n");
	flag = 1;
	pthread_join(thread, NULL);
	msgctl(msgid, IPC_RMID, NULL);
	puts("Программа 2 закончила свою работу.\r\n");
	return 0;
}
