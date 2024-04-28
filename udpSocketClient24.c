#define _WIN32_WINNT _WIN32_WINNT_WIN7 // Windows 7
#include <winsock2.h> // For all socket programming
#include <ws2tcpip.h> // For getaddrinfo, inet_ntop, inet_pton
#include <unistd.h> // For close socket
#include <stdlib.h> // For exit
#include <string.h> // For memset
#include <stdio.h> // For printf


#include <stdint.h> // For integer types
#include <time.h> // For time
#include <math.h> // For maths functions

void OSInit( void ) // Function to initialize the operating system
{
     //Step 1.0
    WSADATA wsaData; //Structure to hold the Windows Socket Implementation Information
    int WSAError = WSAStartup(MAKEWORD(2,0), &wsaData); // Initialize Winsock
    if(WSAError != 0)
    {
        fprintf( stderr,"WSAStartup erno =  %d\n", WSAError); // Print an error message
        exit(-1); // Exit the program
    }
}

void OSCleanup( void ) // Function to cleanup the operating system
{
    //Step 3.0
    WSACleanup(); // Cleanup Winsock
}
#define perror(string) fprintf(stderr, string ": WSA errno = %d\n", WSAGetLastError()) // Define the perror function


//UDP_Client function prototypes
int initializationUDP( struct sockaddr **internet_addresss, socklen_t *internet_address_length ); // Function to initialize the internet address
void executionUDP( int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length); // Function to execute the socket function
void cleanupUDP( int internet_socket, struct sockaddr * internet_address ); // Function to cleanup the socket

//TCP_client function prototypes
int initializationTCP(); // Function to initialize the internet address
void executionTCP( int internet_socket ); // Function to execute the socket function
void cleanupTCP( int internet_socket ); // Function to cleanup the socket


//Main function
int main(int argc, char *argv[])
{

    int random_number = 0; // Variable to hold the random number

    srand(time(NULL)); // Seed the random number generator

    random_number = rand() % 100; // Generate a random number
    //////////////////////////
    // Initialize Winsock/////
    //////////////////////////

    OSInit(); // Initialize the operating system connection

    struct sockaddr * internet_address = NULL; // Variable to hold the internet address
    socklen_t internet_address_length = 0; // Variable to hold the internet address length
    int internet_socketUDP = initializationUDP( &internet_address, &internet_address_length ); // Initialize the internet address

    //Time-out function 16second ==> 16000 ms
    struct timeval timeout; // Variable to hold the timeout
    timeout.tv_sec = 16; // Set the seconds
    timeout.tv_usec = 0; // Set the microseconds
    if(setsockopt(internet_socketUDP, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)<0)) // Set the timeout
    {
        perror("setsockopt"); // Print an error message
    }
    //////////////////////////
    /////// Ececution/////////
    //////////////////////////
    executionUDP( internet_socketUDP, internet_address, internet_address_length); // Execute the socket function
    /////////////////////////// 
    ////////Cleanup///////////
    //////////////////////////

    cleanupUDP( internet_socketUDP, internet_address ); // Cleanup the socket

    WSACleanup(); // Cleanup Winsock
    //End of UDP client

    //TCP client
    printf("\n");
    printf("Start TCP client\n");
    printf("\n");

    OSInit(); // Initialize the operating system connection

    int internet_socketTCP = initializationTCP(); // Initialize the internet address

    executionTCP( internet_socketTCP ); // Execute the socket function

    cleanupTCP( internet_socketTCP ); // Cleanup the socket

    WSACleanup(); // Cleanup Winsock
    //End of TCP client
    
    printf("Compile is works ! \n");
    
return 0;



}

//Function definitions
int initializationUDP( struct sockaddr **internet_addresss, socklen_t *internet_address_length )
{
      //1.1
    struct addrinfo internet_address_setup; // Structure to hold the internet address setup
    struct addrinfo *internet_address_result; // Structure to hold the internet address result
    memset(&internet_address_setup, 0, sizeof(internet_address_setup)); // Initialize the internet address setup
    internet_address_setup.ai_family = AF_UNSPEC; // Set the address family to IPv4
    internet_address_setup.ai_socktype = SOCK_DGRAM; // Set the socket type to stream
    int getaddrinfo_return = getaddrinfo("::1", "24042", &internet_address_setup, &internet_address_result); // Get the internet address information
    if(getaddrinfo_return != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_return)); // Print an error message
        exit(1); // Exit the program
    }

    // Variable to hold the socket
    int internet_socket = -1; 
    struct addrinfo * internet_address_result_iterator = internet_address_result; // Iterator to iterate through the internet address result
    while(internet_address_result_iterator != NULL) // Loop through the internet address result
    {
        //1.2
        internet_socket = socket(internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype,
         internet_address_result_iterator->ai_protocol ); // Create a socket
        if(internet_socket == -1) // If the socket is created
        {
            perror( "socket" ); // Print an error message
        }
        else
        {
            //Step 1.3
            *internet_address_length = internet_address_result_iterator->ai_addrlen; // Set the internet address length
            *internet_addresss = (struct sockaddr *) malloc(*internet_address_length); // Allocate memory for the internet address
            memcpy(*internet_addresss, internet_address_result_iterator->ai_addr, *internet_address_length); // Copy the internet address to the allocated memory
            break; // Break the loop
        }
        internet_address_result_iterator = internet_address_result_iterator->ai_next; // Move to the next internet address result
    }

    freeaddrinfo(internet_address_result); // Free the internet address result

        if(internet_socket == -1) // If the socket is not created
        {
            fprintf(stderr, "socket: no valid socket address foud\n"); // Print an error message
            exit(2); // Exit the program
        }
        return internet_socket; // Return the socket
}

void executionUDP( int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length)
{
    // Initialization of the variables for UDP


    //Step 2.0
    int number_of_bytes_sent = 0;
    //below is the address of the server where data has to be sent
    number_of_bytes_sent = sendto(internet_socket, "Hello, UDP ClientWorld!\n", 32, 0, internet_address, internet_address_length); // Send data to the server
        if(number_of_bytes_sent == -1) // If the data is not sent
            {
                perror("sendto"); // Print an error message
            }
    //Step 2.1
    int number_of_bytes_received = 0; // Variable to hold the number of bytes received
    char buffer[100]; // Buffer to hold the received data
    number_of_bytes_received = recvfrom(internet_socket, buffer, (sizeof(buffer) - 1), 0, internet_address, &internet_address_length); // Receive data from the server
        if(number_of_bytes_received == -1) // If the data is not received
        {
            perror("recvfrom"); // Print an error message
        }
        else
            {
                buffer[number_of_bytes_received] = '\0'; // Null terminate the buffer
                printf("Recived: %s\n", buffer); // Print the received data
            }
}

void cleanupUDP( int internet_socket, struct sockaddr * internet_address )
{
    //Step 3.2
    free(internet_address); // Free the internet address
    //Step 3.1
    closesocket(internet_socket); // Close the socket
}


//TCP client function definitions
int initializationTCP()
{
    //Step 1.0
    struct addrinfo internet_address_setup; // Structure to hold the internet address setup
    struct addrinfo *internet_address_result; // Structure to hold the internet address result
    memset(&internet_address_setup, 0, sizeof(internet_address_setup)); // Initialize the internet address setup
    internet_address_setup.ai_family = AF_UNSPEC; // Set the address family to IPv4
    internet_address_setup.ai_socktype = SOCK_STREAM; // Set the socket type to stream
    int getaddrinfo_return = getaddrinfo("::1", "24042", &internet_address_setup, &internet_address_result); // Get the internet address information
    if(getaddrinfo_return != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_return)); // Print an error message
        exit(1); // Exit the program
    }

    // Variable to hold the socket
    int internet_socket = -1; 
    struct addrinfo * internet_address_result_iterator = internet_address_result; // Iterator to iterate through the internet address result
    while(internet_address_result_iterator != NULL) // Loop through the internet address result
    {
        //1.2
        internet_socket = socket(internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype,
         internet_address_result_iterator->ai_protocol ); // Create a socket
        if(internet_socket == -1) // If the socket is created
        {
            perror( "socket" ); // Print an error message
        }
        else
        {
            //Step 1.3
            int connect_return = connect(internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen); // Connect to the server
            if(connect_return == -1) // If the connection is not established
            {
                perror("connect"); // Print an error message
                closesocket(internet_socket); // Close the socket
            }
            else
            {
                break; // Break the loop
            }
        }
        internet_address_result_iterator = internet_address_result_iterator->ai_next; // Move to the next internet address result
    }

    freeaddrinfo(internet_address_result); // Free the internet address result

        if(internet_socket == -1) // If the socket is not created
        {
            fprintf(stderr, "socket: no valid socket address foud\n"); // Print an error message
            exit(2); // Exit the program
        }
        return internet_socket; // Return the socket
}

void executionTCP( int internet_socket )
{
    //Step 2.0
    int number_of_bytes_sent = 0; // Variable to hold the number of bytes sent
    number_of_bytes_sent = send(internet_socket, "Hello, TCP ClientWorld!\n", 32, 0); // Send data to the server
        if(number_of_bytes_sent == -1) // If the data is not sent
            {
                perror("send"); // Print an error message
            }
    //Step 2.1
    int number_of_bytes_received = 0; // Variable to hold the number of bytes received
    char buffer[100]; // Buffer to hold the received data
    number_of_bytes_received = recv(internet_socket, buffer, (sizeof(buffer) - 1), 0); // Receive data from the server
        if(number_of_bytes_received == -1) // If the data is not received
        {
            perror("recv"); // Print an error message
        }
        else
            {
                buffer[number_of_bytes_received] = '\0'; // Null terminate the buffer
                printf("Recived: %s\n", buffer); // Print the received data
            }

    
}

void cleanupTCP( int internet_socket ) // Function to cleanup the socket
{
    //Step 3.2
    int shutdown_return = shutdown(internet_socket, SD_BOTH); // Shutdown the socket
    if(shutdown_return == -1) // If the socket is not shutdown
    {
        perror("shutdown"); // Print an error message
    }
    //Step 3.1
    close(internet_socket); // Close the socket
}

//End of TCP client

