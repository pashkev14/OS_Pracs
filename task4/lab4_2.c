#include <unistd.h>
#include <stdio.h>
#include <locale.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "RUS");
	puts("Программа 2 начала свою работу.\r\n");
	printf("ID процесса программы 2: %d\r\n", getpid());
	printf("ID родительского процесса программы 2: %d\r\n", getppid());
	pid_t pid;
	pid = fork();
	if (pid == -1) {
		perror("Ошибка создания дочернего процесса");
	}
	else if (pid == 0) {
		int rv = execvp("lab4_1", argv);
		if (rv == -1) {
			perror("Ошибка исполнения дочерней программы");
		}
	}
	else if (pid > 0) {
		printf("ID дочернего процесса программы 2: %d\r\n", pid);
		int status, w;
		while ((w = waitpid(pid, &status, WNOHANG)) == 0) {
			puts("Программа 2 ожидает завершения дочернего процесса\r\n");
			usleep(500000);
		}
		if (w == -1) {
			perror("Ошибка waitpid");
		}
		else {
			printf("Дочерний процесс программы 2 завершился с кодом %d\r\n", WEXITSTATUS(status));
		}
	}
	puts("Программа 2 закончила свою работу.\r\n");
	return 0;
}
