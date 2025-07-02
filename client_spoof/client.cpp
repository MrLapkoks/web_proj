#include <iostream>
#include <ncurses.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>

#define PORT 1234

using namespace std;

int main() {
  int sock = 0;
  char buffer[1024] = {0};
  struct sockaddr_in serv_addr;
  const char *hello = "[login request]username|123";

  // Create socket
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    std::cerr << "Socket creation error" << std::endl;
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // Convert IPv4 and IPv6 addresses from text to binary
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    std::cerr << "Invalid address/ Address not supported" << std::endl;
    return -1;
  }

  // Connect to server
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    std::cerr << "Connection Failed" << std::endl;
    return -1;
  }

  // Send data
  send(sock, hello, strlen(hello), 0);
  std::cout << "Message sent" << std::endl;
  std::cout << "waiting for response" << std::endl;
  read(sock, buffer, 1024);
  close(sock);
  std::cout << "response: " << buffer << std::endl;
  std::string portnum = buffer;
  portnum = portnum.substr(portnum.find("|")+1);
  std::cout << "[" << portnum << "]" << std::endl;
  serv_addr.sin_port = htons(stoi(portnum));
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
      std::cerr << "Connection 2 Failed" << std::endl;
      return -1;
  }
  const char *handshake = "[handshake]";
  send(sock, handshake, strlen(handshake), 0);
  //std::cout << portnum << std::endl;
  std::cout << "Handshake sent" << std::endl;
  std::cout << "waiting for response" << std::endl;
  read(sock, buffer, 1024);
  std::cout << "response: " << buffer << std::endl;

  // Close socket
  close(sock);
  return 0;
}
