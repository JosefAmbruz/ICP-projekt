#include <iostream>
#include <string>
#include <cstring>
#include <print>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 9898

using namespace std;

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        println(stderr, "Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        println(stderr, "Invalid address/Address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        println(stderr, "Connection Failed");
        return -1;
    }

    string input;
    while (true) {
        println("Enter command (STATUS, EVAL <expression>, or QUIT): ");
        getline(cin, input);

        if (input == "QUIT") break;

        send(sock, input.c_str(), input.length(), 0);
        
        int valread = read(sock, buffer, 1024);
        if (valread > 0) {
            buffer[valread] = '\0';
            println("Server response: {}", buffer);
        } else {
            println(stderr, "Error reading from socket");
        }
    }

    close(sock);
    return 0;
}