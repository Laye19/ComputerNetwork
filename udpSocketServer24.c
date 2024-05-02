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

//UDP_Server function prototypes 
int initializationUDP(); // Function to initialize the internet address
void executionUDP( int internet_socket ); // Function to execute the socket function
void cleanupUDP( int internet_socket ); // Function to cleanup the socket

//TCP_Server function prototypes
int initializationTCP(); // Function to initialize the internet address
int connectionTCP( int internet_socket ); // Function to connect to the client
void executionTCP( int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length );// Function to execute the socket function
void cleanupTCP( int internet_socket, int client_internet_socket ); // Function to cleanup the socket


//Main function
int main(int argc, char *argv[])
{
    //////////////////////////
    // Initialize Winsock/////
    //////////////////////////

    OSInit(); // Initialize the operating system
    struct sockaddr * internet_address = 0; // Structure to hold the internet address
    socklen_t internet_address_length = 0; // Variable to hold the internet address length

    int internet_socketUDP = initializationUDP(); // Initialize the internet address
    //////////////////////////
    /////// Ececution/////////
    //////////////////////////
      // Instellen van time-out op 1 seconde

    executionUDP( internet_socketUDP); // Execute the socket function
    /////////////////////////// 
    ////////Cleanup///////////
    //////////////////////////

    cleanupUDP( internet_socketUDP ); // Cleanup the socket

    WSACleanup(); // Cleanup Winsock

    //End UDP Server

    //TCP Server Strat
    printf("\n");
    printf("TCP Server is starting\n");
    printf("\n");

    OSInit(); // Initialize the operating system

    int internet_socketTCP = initializationTCP(); // Initialize the internet address

    while (1)
    {
        int client_internet_socket = connectionTCP( internet_socketTCP ); // Connect to the client

        executionTCP( client_internet_socket, internet_address, internet_address_length); // Execute the socket function

        cleanupTCP( internet_socketTCP, client_internet_socket ); // Cleanup the socket
    }
    WSACleanup(); // Cleanup Winsock

    //End TCP Server
    
    printf("Compile is works ! \n");
    
return 0;

}

//UDP_SERVER Function definitions
int initializationUDP()
{
      //1.1
    struct addrinfo internet_address_setup; // Structure to hold the internet address setup
    struct addrinfo *internet_address_result; // Structure to hold the internet address result
    memset(&internet_address_setup, 0, sizeof(internet_address_setup)); // Initialize the internet address setup
    internet_address_setup.ai_family = AF_UNSPEC; // Set the address family to IPv4
    internet_address_setup.ai_socktype = SOCK_DGRAM; // Set the socket type to stream
    internet_address_setup.ai_flags = AI_PASSIVE; // Set the flags to passive
    int getaddrinfo_return = getaddrinfo(NULL, "24042", &internet_address_setup, &internet_address_result); // Get the internet address information
    if(getaddrinfo_return != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_return)); // Print an error message
        exit(1); // Exit the program
    }

    int internet_socket = -1; // Variable to hold the socket
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
            int bind_return = bind(internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen); // Bind the socket
            if(bind_return == -1) // If the socket is not binded
            {
                close(internet_socket); // Close the socket
                perror("bind"); // Print an error message
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

void executionUDP( int internet_socket )
{

    // Initialize variables
    int targetNumber = rand() % 100; // Random number between 0 and 99
    int closestGuess = -1;
    int closestClient = -1;

     //Step 2.1
    int number_of_bytes_received = 0; // Variable to hold the number of bytes received
    char buffer[100]; // Buffer to hold the received data
    struct sockaddr_storage client_internet_address; // Structure to hold the client internet address
    socklen_t client_internet_address_length = sizeof(client_internet_address); // Variable to hold the internet address length
    number_of_bytes_received = recvfrom(internet_socket, buffer, (sizeof(buffer) - 1), 0, (struct sockaddr *) 
    &client_internet_address, &client_internet_address_length); // Receive data from the server
        if(number_of_bytes_received == -1) // If the data is not received
        {
            perror("recvfrom"); // Print an error message
        }
        else
            {
                buffer[number_of_bytes_received] = '\0'; // Null terminate the buffer
                printf("Recived: %s\n", buffer); // Print the received data
            }

    //Step 2.2
    int number_of_bytes_send = 0;
    //below is the address of the server where data has to be sent
    number_of_bytes_send = sendto(internet_socket, "Hello,UDP ServerWorld!\n", 32, 0, (struct sockaddr *)
     &client_internet_address, client_internet_address_length ); // Send data to the server
        if(number_of_bytes_send == -1) // If the data is not sent
            {
                perror("sendto"); // Print an error message
            }
}

void cleanupUDP( int internet_socket )
{
    //Step 3.1
    close(internet_socket); // Close the socket
}

//TCP_SERVER Function definitions
int initializationTCP()
{
    //1.1
    struct addrinfo internet_address_setup; // Structure to hold the internet address setup
    struct addrinfo *internet_address_result; // Structure to hold the internet address result
    memset(&internet_address_setup, 0, sizeof(internet_address_setup)); // Initialize the internet address setup
    internet_address_setup.ai_family = AF_UNSPEC; // Set the address family to IPv4
    internet_address_setup.ai_socktype = SOCK_STREAM; // Set the socket type to stream
    internet_address_setup.ai_flags = AI_PASSIVE; // Set the flags to passive
    int getaddrinfo_return = getaddrinfo(NULL, "24042", &internet_address_setup, &internet_address_result); // Get the internet address information
    if(getaddrinfo_return != 0) // If the internet address information is not found
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_return)); // Print an error message
        exit(1); // Exit the program
    }

    int internet_socket = -1; // Variable to hold the socket
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
            int bind_return = bind(internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen); // Bind the socket
            if(bind_return == -1) // If the socket is not binded
            {
                close(internet_socket); // Close the socket
                perror("bind"); // Print an error message
            }
            else
            {
                //Step 1.4
                int listen_return = listen(internet_socket, 1); // Listen for the client
                if(listen_return == -1) // If the client is not listened
                {
                    close(internet_socket); // Close the socket
                    perror("listen"); // Print an error message
                }
                else
                {
                    break; // Break the loop
                }
            }
        }
        internet_address_result_iterator = internet_address_result_iterator->ai_next; // Move to the next internet address result
    }
    freeaddrinfo(internet_address_result); // Free the internet address result

    if(internet_socket == -1) // If the socket is not created
    {
        fprintf(stderr, "socket: no valid socket address found\n"); // Print an error message
        exit(2); // Exit the program
    }
    return internet_socket; // Return the socket
}

int connectionTCP( int internet_socket )
{
    //Step 2.1
    struct sockaddr_storage client_internet_address; // Structure to hold the client internet address
    socklen_t client_internet_address_length = sizeof(client_internet_address); // Variable to hold the internet address length
    int client_internet_socket = accept(internet_socket, (struct sockaddr *) &client_internet_address, &client_internet_address_length); // Accept the client
    if(client_internet_socket == -1) // If the client is not accepted
    {
        perror("accept"); // Print an error message
        close(internet_socket); // Close the socket
        exit(3); // Exit the program
    }
    return client_internet_socket; // Return the client socket
}

void executionTCP( int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length )
{

    printf("\n##### Starting TCP Server Execution #####\n\n");

    int random_number, guess; // Variables to hold the random number and the guess
    int number_of_bytes_received = 0; // Variable to hold the number of bytes received
   char buffer[1000]; // Buffer to hold the received data  

     // Generate a random number between 0 and 1000000
    srand(time(NULL));
    random_number = rand() % 1000000; 
    printf("Random number: %d\n", random_number); // Print the random number

    //
    while(1)
  {
    // Receive data from the client
    printf("\nBeguinning round ... good luck!\n");
    number_of_bytes_received = recv(internet_socket, (char *)&guess, sizeof(guess), 0); // Receive data from the client
    if(number_of_bytes_received == -1) // If the data is not received
    {
        perror("recv"); // Print an error message
    }
    /*else
    {
        buffer[number_of_bytes_received] = '\0'; // Null terminate the buffer
        printf("Received guess: %d\n", buffer); // Print the received data
    }
   */
   if(number_of_bytes_received == 0)
   {
         printf("Error receveid data from by client\n");
         break;
   }
   else
    {
         //guess = buffer[0]; // Get the guess from the buffer
          printf("Received guess: %d\n", guess); // Print the received guess
    }
    //compare guess with random number

    if(guess < random_number)
    {
     if(send(internet_socket,"Higher", sizeof("Higher"), 0) == -1)
     {
        perror("Error sending data to client");
        break;
     }

    }  
    else if(guess > random_number)
    {
     
     if(send(internet_socket,"Lower", sizeof("Lower"), 0) == -1)
     {
        perror("Error sending data to client");
        break;
     } 
     // Send data to the client
     }
      else if(guess == random_number)
     {
        if(send(internet_socket,"Correct", sizeof("Correct"), 0) == -1)
        {
            perror("Error sending data to client");
            break;
        }
     }
        else
        {
            if(send(internet_socket,"Error", sizeof("Error"), 0) == -1)
            {
                perror("Error sending data to client");
                break;
        }
        break;
     }
    
 }
  
}

void cleanupTCP( int internet_socket, int client_internet_socket )
{
    //Step 4.1
    close(client_internet_socket); // Close the client socket
    close(internet_socket); // Close the socket
}

