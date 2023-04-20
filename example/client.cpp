#include <arpa/inet.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0]
              << " <server-ip> <server-port> <file-path>\n";
    return 1;
  }

  const char *server_ip = argv[1];
  const uint16_t server_port = std::stoi(argv[2]);
  const char *file_path = argv[3];

  // Открываем файл для чтения
  std::ifstream input_file(file_path, std::ios::binary | std::ios::ate);
  if (!input_file.is_open()) {
    std::cerr << "Failed to open input file\n";
    return 1;
  }

  // Определяем размер файла
  const std::streamsize file_size = input_file.tellg();
  input_file.seekg(0, std::ios::beg);

  // Создаем сокет
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    std::cerr << "Failed to create socket\n";
    return 1;
  }

  // Устанавливаем соединение с сервером
  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(server_ip);
  serv_addr.sin_port = htons(server_port);

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    std::cerr << "Failed to connect to server\n";
    close(sockfd);
    return 1;
  }

  // Отправляем содержимое файла
  char buffer[1024];
  memset(buffer, 0, 1024);
  while (input_file.good()) {
    input_file.read(buffer, sizeof(buffer));
    ssize_t bytes_sent = send(sockfd, buffer, input_file.gcount(), 0);
    if (bytes_sent < 0) {
      std::cerr << "Failed to send data\n";
      close(sockfd);
      return 1;
    }
  }

  // Закрываем сокет и файл
  close(sockfd);
  input_file.close();

  std::cout << "File sent successfully\n";

  return 0;
}
