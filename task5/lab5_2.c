#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <locale.h>
#include <signal.h>
#include <poll.h>
#include <time.h>
#include <fcntl.h>
#define _GNU_SOURCE

sem_t* sem;
int fd;
int enter;
bool flag;
struct pollfd fds;
nfds_t nfds;
struct timespec tv;

void error_exit(const char* msg) {
	perror(msg);
	sem_close(sem);
	sem_unlink("/custom_sem");
	exit(-1);
} 

int main() {
	setlocale(LC_ALL, "RUS");
	puts("Программа 2 начала свою работу.\r\n");
	sem = sem_open("/custom_sem", O_CREAT, 0644, 1);
	if (sem == SEM_FAILED) {
		error_exit("Ошибка создания семафора");
	}
	fd = open("file.txt", O_CREAT | O_WRONLY | O_NONBLOCK | O_APPEND, 0644);
	if (fd == -1) {
		error_exit("Ошибка открытия файла");
	}
	fds.fd = 0;
	fds.events = POLLIN;
	nfds = (nfds_t)(1);
	tv.tv_sec = 1;
	tv.tv_nsec = 0;
	flag = true;
	while (flag == true) {
		sem_wait(sem);
		puts("Программа 2 захватила семафор\r\n");
		for (int i = 0; i < 10; ++i) {
			write(fd, "2", 1);
			putchar('2');
			fflush(stdout);
			sleep(1);
		}
		sem_post(sem);
		puts("\r\nПрограмма 2 отпустила семафор\r\n");
		enter = ppoll(&fds, nfds, &tv, NULL);
		if (enter == 1) {
			flag = false;
		}
		else if (enter == -1) {
			close(fd);
			error_exit("Ошибка ppoll");
		}
	}
	if (close(fd) == -1) {
		error_exit("Ошибка закрытия файла");
	}
	sem_close(sem);
	sem_unlink("/custom_sem");
	puts("Программа 2 закончила свою работу.\r\n");
	return 0;
}
