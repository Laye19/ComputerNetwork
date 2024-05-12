#define _WIN32_WINNT  _WIN32_WINNT_WIN7
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

void OSInit(void) {
    WSADATA wsaData;
    int WSAError = WSAStartup(MAKEWORD(2, 0), &wsaData);
    if (WSAError != 0) {
        fprintf(stderr, "WSAStartup errno = %d\n", WSAError);
        exit(-1);
    }
}

void OSCleanup() {
    WSACleanup();
}

void perror(const char* string) {
    fprintf(stderr, "%s: WSA errno = %d\n", string, WSAGetLastError());
}

int send_http_get_request(const char* ip_address) {
    int http_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (http_socket == INVALID_SOCKET) {
        perror("HTTP socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct hostent* host = gethostbyname(ip_address);
    if (!host) {
        perror("gethostbyname failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
    server_addr.sin_port = htons(80);

    if (connect(http_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("HTTP connect failed");
        exit(EXIT_FAILURE);
    }

    char request[1000];
    sprintf(request, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", ip_address);
    send(http_socket, request, strlen(request), 0);

    return http_socket;
}

void receive_http_response(int http_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received;
    while ((bytes_received = recv(http_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }
}

void handle_http_request(const char* ip_address) {
    int http_socket = send_http_get_request(ip_address);
    receive_http_response(http_socket);
    closesocket(http_socket);
}

int main(int argc, char* argv[]) {
    OSInit();

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <attacker_ip>\n", argv[0]);
        return 1;
    }

    handle_http_request(argv[1]);

    OSCleanup();
    return 0;
}
