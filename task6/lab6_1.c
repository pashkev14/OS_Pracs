#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <locale.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

int flag(0);
char buf[256];
void* addr;
int fd;
size_t mem_size(4096);
const char* mem_name("/custom_shared_memory");
sem_t* writer_sem;
sem_t* reader_sem;
const char* writer_sem_name("/writer_sem");
const char* reader_sem_name("/reader_sem");
pthread_t thread;
struct rusage usage;

void sig_handler(int signo) {
	sem_close(writer_sem);
	sem_unlink(writer_sem_name);
	sem_close(reader_sem);
	sem_unlink(reader_sem_name);
	munmap(addr, mem_size);
	close(fd);
	shm_unlink(mem_name);
	exit(EXIT_FAILURE);
}

int generate_message(char* buf) {
	if (getrusage(RUSAGE_SELF, &usage) == -1) {
		return -1;
	}
	memset(buf, 0, 256);
	int size = sprintf(buf, "Максимальный объем резидентной памяти: %li", usage.ru_maxrss);
	return size;
}

void* proc(void* args) {
	puts("Программа 1: поток записи начал свою работу.\r\n");
	while (flag == 0) {
		puts("Программа 1 захватила семафор записи.\r\n");
		int rv = generate_message(buf);
		if (rv < 0) {
			perror("Программа 1: ошибка генерации сообщения");
		}
		memcpy(addr, buf, rv);
		printf("Сгенерировано и записано сообщение: %s\r\n", buf);
		sem_post(writer_sem);
		puts("\r\nПрограмма 1 освободила семафор записи.\r\n");
		sem_wait(reader_sem);
		puts("Программа 1 ждет освобождения семафора чтения.\r\n");
		sleep(1);
	}
	puts("Программа 1: поток записи закончил свою работу.\r\n");
	pthread_exit(NULL);
}

int main() {
	setlocale(LC_ALL, "RUS");
	signal(SIGINT, sig_handler);
	puts("Программа 1 начала свою работу.\r\n");
	fd = shm_open(mem_name, O_CREAT | O_RDWR, 0644);
	ftruncate(fd, mem_size);
	addr = mmap(0, mem_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	writer_sem = sem_open(writer_sem_name, O_CREAT, 0644, 0);
	reader_sem = sem_open(reader_sem_name, O_CREAT, 0644, 0);
	pthread_create(&thread, NULL, proc, NULL);
	puts("Программа 1 ждет нажатия на клавишу.\r\n");
	getchar();
	puts("Программа 1: клавиша нажата.\r\n");
	flag = 1;
	pthread_join(thread, NULL);
	sem_close(writer_sem);
	sem_unlink(writer_sem_name);
	sem_close(reader_sem);
	sem_unlink(reader_sem_name);
	munmap(0, mem_size);
	close(fd);
	shm_unlink(mem_name);
	puts("Программа 1 закончила свою работу.\r\n");
	return 0;
}
