#define _WIN32_WINNT  _WIN32_WINNT_WIN7
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>

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

//HTTP CLIENT - initialize function
int initialization(int flag)
{
    struct addrinfo internet_address_setup;
    struct addrinfo *internet_address_result;
    memset(&internet_address_setup, 0, sizeof(internet_address_setup)); 
    internet_address_setup.ai_family = AF_INET;
    internet_address_setup.ai_socktype = SOCK_STREAM;
    //internet_address_setup.ai_protocol = AI_PASSIVE;

    if(flag == 1)
    {
        int getaddrinfo_return = getaddrinfo("ip-api.com", "80", &internet_address_setup, &internet_address_result);
        if(getaddrinfo_return != 0)
        {
            fprintf(stderr, "getaddrinfo() failed: %s\n", gai_strerror(getaddrinfo_return)); //print the error message to the standard error stream   
            exit(1);
        }

        int internet_socket_HTTP = -1;
        struct addrinfo *internet_address_result_iterator = internet_address_result;
        while(internet_address_result_iterator != NULL)
        {
            internet_socket_HTTP = socket(internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol);
            if(internet_socket_HTTP == -1)
            {
                perror("socket() failed");
                
            }
            else{
                int connect_result = connect(internet_socket_HTTP, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen);
                if(connect_result == -1)
                {
                    perror("connect() failed");
                    close(internet_socket_HTTP);
                } else {
                    break;
                }
            }
            internet_address_result_iterator = internet_address_result_iterator->ai_next;
        }
        freeaddrinfo(internet_address_result);

        if(internet_socket_HTTP == -1)
        {
            fprintf(stderr, "Could not connect to the server\n");
            exit(1);
        }
        return internet_socket_HTTP;
    }
    return -1;
}



//HTTP CLIENT - cleanup function
void cleanup(int client_internet_socket)
{
    int shutdown_return = shutdown(client_internet_socket, SD_SEND);
    if(shutdown_return == -1)
    {
        perror("shutdown() failed");
        //exit(EXIT_FAILURE);
    }
    close(client_internet_socket);
}


