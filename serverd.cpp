#include "serverd.h"

#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFF_SIZE 1024
#define LOCALHOST "127.0.0.1"
const int kPort = 8080;
const char *kHomeDir = std::getenv("HOME");
const std::string kFileName = std::string(kHomeDir) + "/output.txt";

void signal_handler(int signum) {
  if (signum == SIGTERM || signum == SIGHUP) {
    // Закрываем все открытые дескрипторы
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    exit(EXIT_SUCCESS);
  }
}

void demonize() {
  // Создание нового процесса
  pid_t pid = fork();

  // Если не удалось создать новый процесс, выводим ошибку и выходим
  if (pid < 0) {
    std::cerr << "Failed to fork\n";
    exit(EXIT_FAILURE);
  }

  // Если это родительский процесс, завершаем его
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  // Создаем новую сессию
  if (setsid() < 0) {
    std::cerr << "Failed to create new session\n";
    exit(EXIT_FAILURE);
  }

  // Изменяем текущий рабочий каталог на корневой
  if (chdir("/") < 0) {
    std::cerr << "Failed to change working directory\n";
    exit(EXIT_FAILURE);
  }

  // Закрываем стандартные потоки ввода/вывода/ошибок
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  // Открываем /dev/null в качестве стандартных потоков
  open("/dev/null", O_RDONLY);
  open("/dev/null", O_WRONLY);
  open("/dev/null", O_WRONLY);
}

void handle_client(int client_socket, int file) {
  char buffer[BUFF_SIZE];
  int bytes_received;
  while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) >
         0) {
    write(file, buffer, bytes_received);
  }
  close(client_socket);
}

void init_server() {
  demonize();

  signal(SIGTERM, signal_handler);
  signal(SIGHUP, signal_handler);

  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0) {
    std::cerr << "create_socket_error" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << "server socket is create" << std::endl;
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(kPort);
  server_address.sin_addr.s_addr = inet_addr(LOCALHOST);

  if (bind(server_socket, (struct sockaddr *)&server_address,
           sizeof(server_address)) < 0) {
    std::cerr << "bind error" << std::endl;
    close(server_socket);
    exit(EXIT_FAILURE);
  }

  if (listen(server_socket, 10) < 0) {
    std::cerr << "listen_error" << std::endl;
    close(server_socket);
    exit(EXIT_FAILURE);
  }

  while (true) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int client_socket = accept(server_socket, (struct sockaddr *)&client_addr,
                               &client_addr_len);
    if (client_socket < 0) {
      std::cerr << "Error accepting client connection" << std::endl;
      continue;
    }
    int file = open(kFileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (file < 0) {
      std::cerr << "Error opening file" << std::endl;
      exit(EXIT_FAILURE);
    }

    handle_client(client_socket, file);
  }
}

int main() { init_server(); }
