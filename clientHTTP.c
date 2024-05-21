#define _WIN32_WINNT  _WIN32_WINNT_WIN7
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
#define perror(string) fprintf(stderr, "%s: WSA errno = %d\n", string, WSAGetLastError())

//function prototypes
int initialization(int flag);
void cleanup(int client_internet_socket);
void IPgetResuest(const char *client_address_length, FILE *file_ptr, char thread_id_str[30])
{
    int internet_socket_HTTP = initialization(1); //initialize a socket to connect to the API

    //Log the API address
    fputs("Thread ID: ", file_ptr);
    fputs(thread_id_str, file_ptr);
    fputs("The IP address is: ", file_ptr);
    fputs(client_address_length, file_ptr);
    fputs("\n", file_ptr);

    char buffer[1000];
    char request[100] = {0};

    //Create the HTTP request
    sprintf(request, "GET /json/%s HTTP/1.0\r\nHost: ip-api.com\r\nConnection: close\r\n", client_address_length);
    printf("HTTP Request: %s\n", request);

    //Send the HTTP request
    int num_bytes_sent = send(internet_socket_HTTP, request, strlen(request), 0);
    if(num_bytes_sent == -1)
    {
        perror("send() failed");
        //exit(EXIT_FAILURE);
    }

    //Receive the HTTP response
    int num_bytes_received = recv(internet_socket_HTTP, buffer, sizeof(buffer) - 1, 0);
    if(num_bytes_received == -1)
    {
        perror("recv() failed");
        //exit(EXIT_FAILURE);
    }
    else
    {
        buffer[num_bytes_received] = '\0';
        printf("HTTP Response: %s\n", buffer);
    }

    
    char *jsonFile = strstr(buffer, "{"); //find the start of the JSON data in the HTTP response
    if(jsonFile == NULL)
    {
        num_bytes_received = recv(internet_socket_HTTP, buffer, sizeof(buffer) - 1, 0);
        if(num_bytes_received == -1)
        {
            perror("recv() failed");
            //exit(EXIT_FAILURE);
        }
        else
        {
            buffer[num_bytes_received] = '\0'; //null-terminate the buffer
            printf("HTTP Response: %s\n", buffer); //print the HTTP response
        }

        //log the geo-location data
        fputs("Thread ID: ", file_ptr); //log the thread ID to the file pointer
        fputs(thread_id_str, file_ptr); //log the thread ID to the file pointer 
        fputs("Geolocation =  ", file_ptr); //log the geo-location data to the file pointer 
        fputs(buffer, file_ptr); //log the geo-location data to the file pointer
        fputs("\n", file_ptr); //log the geo-location data to the file pointer
    }
    else{
        //log the geo-location data
        fputs("Thread ID: ", file_ptr); //log the thread ID to the file pointer
        fputs(thread_id_str, file_ptr); //log the thread ID to the file pointer 
        fputs("Geolocation =  ", file_ptr); //log the geo-location data to the file pointer 
        fputs(jsonFile, file_ptr); //log the geo-location data to the file pointer
        fputs("\n", file_ptr); //log the geo-location data to the file pointer
    }
    //cleanup the socket
    cleanup(internet_socket_HTTP);
}

/*
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
*/

int initialization(int flag)
{
    int internet_socket_HTTP = socket(AF_INET, SOCK_STREAM, 0); //create a socket to connect to the API
    if(internet_socket_HTTP == -1)
    {
        perror("socket() failed");
        //exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address_HTTP = {0}; //initialize the server address structure
    server_address_HTTP.sin_family = AF_INET; //set the address family to AF_INET
    server_address_HTTP.sin_port = htons(80); //set the port to 80

    struct hostent *host = gethostbyname("ip-api.com"); //get the IP address of the API
    if(host == NULL)
    {
        perror("gethostbyname() failed");
        //exit(EXIT_FAILURE);
    }

    server_address_HTTP.sin_addr.s_addr = *(long*)host->h_addr_list[0]; //set the IP address of the API

    if(connect(internet_socket_HTTP, (struct sockaddr *)&server_address_HTTP, sizeof(server_address_HTTP)) == -1) //connect to the API
    {
        perror("connect() failed");
        //exit(EXIT_FAILURE);
    }

    return internet_socket_HTTP;

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
