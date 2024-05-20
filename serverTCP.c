
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

    //#define LOG_FILE "log.txt"
    #define PORT "22"
    #define ATTACK_DATA "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed nonummy."

    #include <pthread.h> // for pthread_create, pthread_join 

    struct ThreadArgs
    {
        int internet_socket;
        FILE *log_file;
        char client_internet_string[INET6_ADDRSTRLEN];
    };
    //function prototype
    int initialization(int flag);
    int connection(int internet_socket, const char* client_address_string, int size); //connect to the client 
    void IPgetRequest(const char * client_address_string, FILE *log_file,char thread_id_str[10]); //get the IP address of the client 
    void execution(int internet_socket, FILE *log_file, char client_internet_string[INET6_ADDRSTRLEN]);
    void cleanup(int client_internt_socket );
    void sub_executionThread(void* arg);

    int main(int argc, char *argv[])
    {
        FILE *log_file = fopen(log_file, "w");
        if(log_file == NULL)
        {
            perror("fopen");
            exit(1);
        }

        OSInit();//initialize the winsock library 

        int internet_socket = initialization(0); //initialize the internet socket 

        char client_address_string[INET6_ADDRSTRLEN];

        while(1)
        {
            int client_internet_socket = connection(internet_socket, client_address_string, sizeof(client_address_string)); //connect to the client 
            pthread_t thread; //initialize the thread 

            //allocate memory for the thread_data struct 
            struct ThreadArgs* data = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));

            data->internet_socket = client_internet_socket; //initialize the internet socket 
            data->log_file = log_file; //initialize the log file 

            strcpy(data->client_address, client_address_string); //copy the client address to the thread data struct 

            //create the thread
            int thread_create_data = pthread_create(&thread, NULL, sub_executionThread, data);
            if(thread_create_data != 0)
            {
                perror("pthread_create");
                exit(1);
            }

        
        }

        fclose(log_file); //close the log file 
        
        //excution(client_internt_socket);
        cleanup(internet_socket); //cleanup the internet socket

        OSCleanup();
        return 0;
    }

    int initialization(int flag)
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

    int connection(int internet_socket, const char* client_address_string, int size)
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

    void cleanup(int client_internt_socket )
    {
        close(client_internt_socket);
        //close(internet_socket);
    }

