#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <iostream>
#include <queue>
#include <string>

int listener_id;
int client_id;
std::queue<std::string> msglist;
int receiver_flag = 0;
int proccessor_flag = 0;
int connector_flag = 0;
pthread_t receiver_thread;
pthread_t proccessor_thread;
pthread_t connector_thread;
pthread_mutex_t s_mutex;
rusage usage;

void error_exit(const char* msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

void finisher() {
	receiver_flag = 1;
    proccessor_flag = 1;
    connector_flag = 1;
    pthread_join(receiver_thread, NULL);
    pthread_join(proccessor_thread, NULL);
    pthread_join(connector_thread, NULL);
    if (shutdown(client_id, 2) == -1) {
        perror("Ошибка закрытия соединения с рабочим сокетом");
    }
    pthread_mutex_destroy(&s_mutex);
    close(client_id);
    close(listener_id);
    std::cout << "Программа-сервер закончила работу.\r\n";
}

void sig_handler(int signo) {
    printf("Программа-сервер получила сигнал с кодом %d\r\n", signo);
    sleep(1);
    if (signo == SIGINT || signo == SIGTERM || signo == SIGPIPE) {
        finisher();
        exit(EXIT_SUCCESS);
    }
}

int generate_message(char* buf) {
	if (getrusage(RUSAGE_SELF, &usage) == -1) {
		return -1;
	}
	memset(buf, 0, 256);
	int size = sprintf(buf, "Максимальный объем резидентной памяти: %li", usage.ru_maxrss);
	return size;
}

void* receiver(void* args) {
	std::cout << "Программа-сервер: поток приема запросов начал работу.\r\n";
	char buf[256];
	while (receiver_flag == 0) {
		memset(buf, 0, 256);
		int reccount = recv(client_id, buf, 256, 0);
		if (reccount == -1) {
			perror("Ошибка приема данных из рабочего сокета");
			sleep(1);
		}
		else if (reccount == 0) {
			sleep(1);
		}
		else {
			pthread_mutex_lock(&s_mutex);
			printf("Программа-сервер получила запрос от клиента: %s\r\n", buf);
			msglist.emplace(buf);
			pthread_mutex_unlock(&s_mutex);
		}
	}
	std::cout << "Программа-сервер: поток приема запросов закончил работу.\r\n";
	pthread_exit(NULL);
}

void* proccessor(void* args) {
	std::cout << "Программа-сервер: поток обработки запросов и передачи ответов начал работу.\r\n";
	char buf[256];
	while (proccessor_flag == 0) {
		pthread_mutex_lock(&s_mutex);
		if (msglist.empty() == 0) {
			auto message = msglist.front();
			msglist.pop();
			pthread_mutex_unlock(&s_mutex);
			memset(buf, 0, 256);
			int msgsize = generate_message(buf);
			if (msgsize == -1) {
				perror("Ошибка генерации сообщения");
			}
			if (send(client_id, buf, 256, 0) == -1) {
                perror("Ошибка передачи данных в рабочий сокет");
            }
            else {
                printf("Программа-сервер послала клиенту сообщение: %s\r\n", buf);
            }
		}
		else {
			std::cout << "Очередь запросов пуста!\r\n";
			pthread_mutex_unlock(&s_mutex);
			sleep(1);
		}
	}
	std::cout << "Программа-сервер: поток обработки запросов и передачи ответов закончил работу.\r\n";
	pthread_exit(NULL);
}

void* connector(void* args) {
	std::cout << "Программа-сервер: поток ожидания соединений начал работу.\r\n";
	char buf[256];
	while (connector_flag == 0) {
		sockaddr_un addr;
		int addr_len = sizeof(addr);
		client_id = accept(listener_id, (sockaddr*)&addr, (socklen_t*)&addr_len);
		if (client_id == -1) {
			perror("Ошибка приема соединения от клиента");
            sleep(1);
		}
		else {
			std::cout << "Программа-сервер: поток ожидания соединений получил прием соединения от клиента.\r\n";
			pthread_create(&receiver_thread, NULL, receiver, NULL);
            pthread_create(&proccessor_thread, NULL, proccessor, NULL);
            std::cout << "Программа-сервер: поток ожидания соединений закончил работу.\r\n";
            pthread_exit(NULL);
		}
	}
	std::cout << "Программа-сервер: поток ожидания соединений закончил работу.\r\n";
	pthread_exit(NULL);
}

int main() {
	setlocale(LC_ALL, "RUS");
	signal(SIGPIPE, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
	std::cout << "Программа-сервер начала работу.\r\n";
	listener_id = socket(AF_UNIX, SOCK_STREAM, 0);
	if (listener_id == -1) {
        error_exit("Ошибка создания слушающего сокета");
    }
    fcntl(listener_id, F_SETFL, O_NONBLOCK);
    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    sprintf(addr.sun_path, "server_socket.soc");
    if (bind(listener_id, (sockaddr*)&addr, sizeof(addr)) == -1) {
        error_exit("Ошибка привязки слушающего сокета к адресу");
    }
    int optval = 1;
    if (setsockopt(listener_id, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        error_exit("Ошибка задания свойств слушающему сокету");
    }
    if (listen(listener_id, 1) == -1) {
        error_exit("Ошибка перевода слушающего сокета в состояние прослушивания");
    }
    int addr_len = sizeof(addr);
    printf("Адрес сокета сервера: %d\r\n", getsockname(listener_id, (sockaddr*)&addr, (socklen_t*)&addr_len));
    pthread_mutex_init(&s_mutex, NULL);
    pthread_create(&connector_thread, NULL, connector, NULL);
    std::cout << "Программа ждет нажатия на клавишу.\r\n";
    getchar();
    std::cout << "Программа-сервер: клавиша нажата.\r\n";
    finisher();
	return 0;
}
