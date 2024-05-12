
    #define _WIN32_WINNT  _WIN32_WINNT_WIN7
    #include <winsock2.h> // for all socket programming 
    #include <ws2tcpip.h> // for getaddrinfo, inet_ntop, inet_pton
    #include <stdio.h> // for printf, fprintf, perror
    #include <unistd.h> // for close
    #include <string.h> // for memset
    #include <stdlib.h> // for exit
    #include <stdint.h> // for uint16_t, uint32_t
    void OSInit( void )
    {
        WSADATA wsaData;
        int WSAError = WSAStartup(MAKEWORD(2, 0), &wsaData);
        if(WSAError != 0)
        {
            fprintf(stderr, "WSAStartup errno = %d\n", WSAError);
            exit(-1);
        }
    }
    void OSCleanup()
    {
        WSACleanup();
    }
    #define perror(string) fprintf(stderr, string ": WSA errno = %d\n", WSAGetLastError())

    #define LOG_FILE "UnoReverse"
    #define PORT "22"
    #define ATTACK_DATA "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed nonummy."
    //function prototype
    int initialization();
    int connection(int internet_socket );
    void excution(int internet_socket );
    void cleanup(int internet_socket, int client_internt_socket );

    int main(int argc, char *argv[])
    {

        OSInit();

        int internet_socket = initialization();
        int client_internt_socket = connection(internet_socket);
        excution(client_internt_socket);
        cleanup(internet_socket, client_internt_socket);

        OSCleanup();
        return 0;
    }

    int initialization()
    {
        struct addrinfo internet_address_setup;
        struct addrinfo *internet_address_result;
        memset(&internet_address_setup, 0, sizeof(internet_address_setup));
        internet_address_setup.ai_family = AF_UNSPEC;
        internet_address_setup.ai_socktype = SOCK_STREAM;
        internet_address_setup.ai_flags = AI_PASSIVE;
        int getaddrinfo_return = getaddrinfo("NULL", PORT, &internet_address_setup, &internet_address_result);
        if(getaddrinfo_return != 0)
        {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_return));
            exit(1);
        }

        int internet_socket = -1;
        struct addrinfo *internet_address_result_iterator = internet_address_result;
        while(internet_address_result_iterator != NULL)
        {
            internet_socket = socket(internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol);
            if(internet_socket == -1)
            {
                perror("socket");
            }
            else
            {
                int bind_resultaat = bind(internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen);
                if(bind_resultaat == -1)
                {
                    close(internet_socket);
                    perror("bind");
                }
                else
                {
                   int listen_return = listen(internet_socket, 10);
                    if(listen_return == -1)
                    {
                        close(internet_socket);
                        perror("listen");
                    }
                    else
                    {
                        break;
                    }
                }
            }
            internet_address_result_iterator = internet_address_result_iterator->ai_next;
        }
        freeaddrinfo(internet_address_result);

        if(internet_socket == -1)
        {
            fprintf(stderr, "socket: no valid socket address found\n" );
            exit(2);
        }
        return internet_socket;
    }

    int connection(int internet_socket )
    {
        struct sockaddr_storage client_address;
        socklen_t client_address_size = sizeof(client_address);
        int client_internt_socket = accept(internet_socket, (struct sockaddr *)&client_address, &client_address_size);
        if(client_internt_socket == -1)
        {
            perror("accept");
            close(internet_socket);
            exit(3);
        }
        return client_internt_socket;
    }

    void excution(int internet_socket)
    {
        char buffer[1000];
        sprintf(buffer, "Reverse attack data: %s\n", ATTACK_DATA);
        send(internet_socket, buffer, strlen(buffer), 0);

    }

    void cleanup(int internet_socket, int client_internt_socket )
    {
        close(client_internt_socket);
        close(internet_socket);
    }

