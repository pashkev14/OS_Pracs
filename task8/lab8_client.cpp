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
#include <string>

int server_id;
int sender_flag = 0;
int receiver_flag = 0;
int connector_flag = 0;
pthread_t sender_thread;
pthread_t receiver_thread;
pthread_t connector_thread;

void error_exit(const char* msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

void finisher() {
	sender_flag = 1;
	receiver_flag = 1;
    connector_flag = 1;
    pthread_join(sender_thread, NULL);
    pthread_join(receiver_thread, NULL);
    pthread_join(connector_thread, NULL);
    if (shutdown(server_id, 2) == -1) {
        perror("Ошибка закрытия соединения с рабочим сокетом");
    }
    unlink("client_socket.soc");
    close(server_id);
    std::cout << "Программа-клиент закончила работу.\r\n";
}

void sig_handler(int signo) {
    printf("Программа-клиент получила сигнал с кодом %d\r\n", signo);
    sleep(1);
    if (signo == SIGINT || signo == SIGTERM || signo == SIGPIPE) {
        finisher();
        exit(EXIT_SUCCESS);
    }
}

void* sender(void* args) {
	std::cout << "Программа-клиент: поток передачи запросов начал работу.\r\n";
	char buf[256];
	int id = 0;
	while (sender_flag == 0) {
		memset(buf, 0, 256);
		sprintf(buf, "ID запроса: %d", id++);
		int sentcount = send(server_id, buf, 256, 0);
		if (sentcount == -1) {
		    perror("Ошибка посылки запроса на сервер");
        }
        else {
            printf("Программа-клиент послала запрос на сервер: %s\r\n", buf);
        }
        sleep(1);
	}
	std::cout << "Программа-клиент: поток передачи запросов закончил работу.\r\n";
	pthread_exit(NULL);
}

void* receiver(void* args) {
	std::cout << "Программа-клиент: поток приема ответов начал работу.\r\n";
	char buf[256];
	while (receiver_flag == 0) {
		memset(buf, 0, 256);
		int reccount = recv(server_id, buf, 256, 0);
		if (reccount == -1) {
			perror("Ошибка приема ответа от сервера");
			sleep(1);
		}
		else if (reccount == 0) {
			sleep(1);
		}
		else {
			printf("Программа-клиент получила ответ от сервера: %s\r\n", buf);
		}
	}
	std::cout << "Программа-клиент: поток приема ответов закончил работу.\r\n";
	pthread_exit(NULL);
}

void* connector(void* args) {
	std::cout << "Программа-клиент: поток установления соединения начал работу.\r\n";
	sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    sprintf(addr.sun_path, "server_socket.soc");
    while (connector_flag == 0) {
    	if (connect(server_id, (sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("Ошибка установления соединения с сервером");
            sleep(1);
        }
        else {
        	std::cout << "Программа-клиент: поток установил соединение с сервером.\r\n";
            pthread_create(&sender_thread, NULL, sender, NULL);
            pthread_create(&receiver_thread, NULL, receiver, NULL);
            std::cout << "Программа-клиент: поток установления соединения закончил работу.\r\n";
            pthread_exit(NULL);
        }
    }
    std::cout << "Программа-клиент: поток установления соединения закончил работу.\r\n";
    pthread_exit(NULL);
}

int main() {
	setlocale(LC_ALL, "RUS");
	signal(SIGPIPE, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
	std::cout << "Программа-клиент начала работу.\r\n";
	server_id = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_id == -1) {
        error_exit("Ошибка создания сокета для работы с сервером");
    }
    fcntl(server_id, F_SETFL, O_NONBLOCK);
    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    sprintf(addr.sun_path, "client_socket.soc");
    if (bind(server_id, (sockaddr*)&addr, sizeof(addr)) == -1) {
        error_exit("Ошибка привязки сокета к адресу");
    }
    std::cout << "Сокет для работы с сервером был успешно привязан к адресу.\r\n";
    int addr_len = sizeof(addr);
    getsockname(server_id, (sockaddr*)&addr, (socklen_t*)&addr_len);
    printf("Адрес сокета: %s\r\n", addr.sun_path);
    pthread_create(&connector_thread, NULL, connector, NULL);
	std::cout << "Программа ждет нажатия на клавишу.\r\n";
    getchar();
    std::cout << "Программа-клиент: клавиша нажата.\r\n";
    finisher();
	return 0;
}
