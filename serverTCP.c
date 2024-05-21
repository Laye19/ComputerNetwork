
    #define _WIN32_WINNT  _WIN32_WINNT_WIN7
    #include <winsock2.h> // for all socket programming 
    #include <ws2tcpip.h> // for getaddrinfo, inet_ntop, inet_pton
    #include <stdio.h> // for printf, fprintf, perror
    #include <unistd.h> // for close
    #include <string.h> // for memset
    #include <stdlib.h> // for exit
    #include <stdint.h> // for uint16_t, uint32_t

    // Link with ws2_32.lib
    #pragma comment(lib, "Ws2_32.lib")

   
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
    #define IPV6_V6ONLY 23 
    //#define ATTACK_DATA "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed nonummy."

    #include <pthread.h> // for pthread_create, pthread_join 

    struct Thread_data{
        int internet_socket_TCP;
        FILE* file_ptr;
        char client_address_length[INET6_ADDRSTRLEN]; //INET6_ADDRSTRLEN is the maximum length of an IPv6 address
    };
    //function prototype
    int initialization(int flag);
    int connection(int internet_socket_TCP, const char* client_address_length, int size); //connect to the client 
    void IPgetRequest(const char * client_address_length, FILE *file_ptr,char thread_id_str[30]); //get the IP address of the client 
    void execution(int internet_socket, FILE *file_ptr, char client_internet_length[INET6_ADDRSTRLEN]);// execute the server 
    void cleanup(int client_internet_socket ); // cleanup the server
    void sub_executionThread(void* arg); //execute the thread subroutine function

    int main(int argc, char *argv[])
    {
        struct Thread_data data;
        data.internet_socket_TCP = -1;
        data.file_ptr = NULL;

        const char* log_file = "logfile.log";
        FILE *file_ptr = fopen(log_file, "w");
        if(file_ptr == NULL)
        {
            perror("fopen");
            exit(1);
        }

        OSInit();//initialize the winsock library 

        int internet_socket_TCP = initialization(0); //initialize the internet socket 

        char client_address_length[INET6_ADDRSTRLEN];

        while(1)
        {
            int client_internet_socket = connection(internet_socket_TCP, client_address_length, sizeof(client_address_length)); //connect to the client 
            pthread_t thread; //initialize the thread 

            //allocate memory for the thread_data struct 
            struct Thread_data* data = (struct Thread_data*)malloc(sizeof(struct Thread_data));

            data->internet_socket_TCP = client_internet_socket; //initialize the internet socket 
            data->file_ptr = file_ptr; //initialize the log file 

            strcpy(data->client_address_length, client_address_length); //copy the client address to the thread data struct 

            //create the new thread to handle the client request
            int thread_create_data = pthread_create(&thread, NULL, sub_executionThread, data);
            if(thread_create_data != 0)
            {
                fprintf(stderr, "Error creating thread: %d\n", thread_create_data);
            }        
        }

        fclose(file_ptr); //close the log file 

        cleanup(internet_socket_TCP); //cleanup the internet socket

        OSCleanup();

        return 0;
    }
    //TCP server initialization
    int initialization(int flag)  //flag = 0 for server initialization and flag = 1 for client initialization
    {
        struct addrinfo internet_address_setup;
        struct addrinfo *internet_address_result;
        memset(&internet_address_setup, 0, sizeof(internet_address_setup));
        internet_address_setup.ai_family = AF_UNSPEC;
        internet_address_setup.ai_socktype = SOCK_STREAM;
        internet_address_setup.ai_flags = AI_PASSIVE;

        if(flag == 0) //server initialization
        {
            int getaddrinfo_return = getaddrinfo("NULL", PORT, &internet_address_setup, &internet_address_result);
            if(getaddrinfo_return != 0)
            {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_return));
                exit(1);
            }

            int internet_socket_TCP = -1;
            int iResult = 0;
            struct addrinfo *internet_address_result_iterator = internet_address_result;
            while(internet_address_result_iterator != NULL)
            {
                //Step 1.2: Create a socket
                internet_socket_TCP = socket(internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol);
                int ipv6only = 0;
                iResult = setsockopt(internet_socket_TCP, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&ipv6only, sizeof(ipv6only)); 
                if(internet_socket_TCP == -1)
                {
                    perror("socket");
                }
                else
                {
                    int bind_resultaat = bind(internet_socket_TCP, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen);
                    if(bind_resultaat == -1)
                    {
                        perror("bind");
                        close(internet_socket_TCP);
                    }
                    else
                    {
                    int listen_return = listen(internet_socket_TCP, 1);
                        if(listen_return == -1)
                        {  
                            close(internet_socket_TCP); // close the socket if the listen function fails
                            perror("listen");
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                internet_address_result_iterator = internet_address_result_iterator->ai_next; //move to the next address in the linked list 
            }
            freeaddrinfo(internet_address_result);

            if(internet_socket_TCP == -1)
                 {
                fprintf(stderr, "socket: no valid socket address found\n" );
                exit(2);
                 }
            return internet_socket_TCP;
        }
        return -1;
    }

    //TCP server connection function
    int connection(int internet_socket_TCP, const char* client_address_length, int size)
    {
        struct sockaddr_storage client_address;
        socklen_t client_address_size = sizeof(client_address);
        int client_internet_socket = accept(internet_socket_TCP, (struct sockaddr *)&client_address, &client_address_size);
        if(client_internet_socket == -1)
        {
            perror("accept");
            close(internet_socket_TCP);
            exit(3);
        }
        
        //get the IP address of the client
        if(client_address.ss_family == AF_INET6)
        {
            struct sockaddr_in *client_address_ipv4 = (struct sockaddr_in *)&client_address; //cast the client address to an IPv4 address 
            inet_ntop(client_address.ss_family, &client_address_ipv4->sin_addr, client_address_length, size); //convert the client address to a string 
        }
        else
        {
            struct sockaddr_in6 *client_address_ipv6 = (struct sockaddr_in6 *)&client_address; //cast the client address to an IPv6 address 
            inet_ntop(client_address.ss_family, &client_address_ipv6->sin6_addr, client_address_length, size); //convert the client address to a string 
        }
        printf("Client IP address: %s\n", client_address_length); //print the client address to the console 
        return client_internet_socket;
    }

    //TCP server execution function 
    void execution(int internet_socket_TCP, FILE *file_ptr, char client_internet_length[INET6_ADDRSTRLEN])
    {
        //Take the thread_id and convert it to a string
        pthread_t thread_id = pthread_self();
        char thread_id_str[30]; //initialize the thread_id string 
        sprintf(thread_id_str, "%ld", thread_id); //convert the thread_id to a string 

        //Received message from the client
        int number_of_bytes_received = 0;
        char buffer[1000];

        number_of_bytes_received = recv(internet_socket_TCP, buffer, sizeof(buffer)- 1, 0); //receive the message from the client 
        if(number_of_bytes_received == -1)
        {
            perror("recv");
            exit(4);
        }
        else{
            buffer[number_of_bytes_received] = '\0'; //add a null terminator to the end of the buffer
            printf("Received message: %s\n", buffer); //print the message to the console
        }
        //cALL THE HTTP function to get IP geolocation
        IPgetRequest(client_internet_length, file_ptr, thread_id_str);
        
        //write the thread ID to the log file
        fputs("Thread ID:", file_ptr); 
        fputs(thread_id_str, file_ptr);
        fputs("Client Message: ", file_ptr);
        fputs(buffer, file_ptr); //write the client message to the log file 
        fputs("\n", file_ptr); //write a newline character to the log file

        //step 3.1: send the message back to the client
        int number_of_bytes_send = 0;
        int total_number_of_bytes_send = 0;

        char total_of_bytes_send_str[30];

        while(1)
        {
            number_of_bytes_send = send(internet_socket_TCP,"Hello Programming World!\n",25, 0); //send the message back to the client 
            if(number_of_bytes_send == -1)
            {
                printf("Client disconnected unexpectedly. Total bytes transmitted: %d\n", total_number_of_bytes_send);
                sprintf(total_of_bytes_send_str, "%d", total_number_of_bytes_send);
                fputs("Thread ID: ", file_ptr);
                fputs(thread_id_str, file_ptr);
                fputs(" Total bytes sent = ", file_ptr);
                fputs(total_of_bytes_send_str, file_ptr);
                fputs("\n", file_ptr);
                break;
            }
            else 
            {
                total_number_of_bytes_send += number_of_bytes_send;
                sleep(1); //Just to slow down the server
            }
           
        }
        cleanup(internet_socket_TCP); //cleanup the internet socket

    }

    void cleanup(int client_internet_socket )
    {
        //step 4: close the socket
        #ifdef _WIN32 //for windows
            int shutdown_return = shutdown(client_internet_socket, SD_RECEIVE);
            if(shutdown_return == -1)
            {
                perror("shutdown");
            }
        #endif
        close(client_internet_socket);
        //close(internet_socket);
    }
    //subroutine function for the thread 
    void sub_executionThread(void* arg)
    {
        struct Thread_data* data = (struct Thread_data *)arg; //cast the argument to a thread_data struct

        //thread execution function for each client request
        execution(data->internet_socket_TCP, data->file_ptr, data->client_address_length);//call the execution function for each client request

        //free the memory allocated for the thread_data struct
        free(data);
        pthread_exit(NULL); //exit the thread 
    }


