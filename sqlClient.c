#include <stdio.h> // Include the standard input/output header for input and output operations
#include <sys/socket.h> // Include the socket header for creating, connecting, and communicating with sockets
#include <arpa/inet.h> // Include the internet protocol header for handling IP addresses
#include <unistd.h> // Include the unistd header for various system calls, such as read and write
#include <string.h> // Include the string header for string manipulation functions

#define PORT 8080 // Define the port number for the server

int valread; // Declare a variable to store the number of bytes read from the socket

int main() {
    int sock = 0; // Declare a socket descriptor
    struct sockaddr_in serv_addr; // Declare a structure to hold the server's address information
    char buffer[1024] = {0}; // Declare a buffer to store data received from the server

    // Create a socket for communication
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error"); // Print an error message if socket creation fails
        return -1; // Return an error code
    }

    // Initialize the server address structure
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // Set the address family to IPv4
    serv_addr.sin_port = htons(PORT); // Set the port number to the defined port

    // Convert the server IP address from string to binary format
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported\n"); // Print an error message if the IP address is invalid
        return -1; // Return an error code
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n"); // Print an error message if the connection fails
        return -1; // Return an error code
    }

    // Main loop for client-server communication
    while (1) {
        printf("Enter command\n"
               "login username password\n"
               "register username password)\n"
               "or exit\n"
               ":>"); // Prompt the user to enter a command

        scanf("%s", buffer); // Read the command from the user

        // Append username and password to the buffer
        // Append a space to the end of the buffer to separate the command 
        //from the username
        strcat(buffer, " "); 
		
		// Read the username and store it in the buffer, 
		//starting from the position after the space
		scanf("%s", buffer + strlen(buffer)); 
		
		// Append another space to separate the username from the password
		strcat(buffer, " "); 

		// Read the password and store it in the buffer, starting from 
		//the position after the second space
		scanf("%s", buffer + strlen(buffer)); 

        // Send the command to the server
        send(sock, buffer, strlen(buffer), 0);

        // Receive the server's response
        valread = read(sock, buffer, 1024);
        printf("Server: %s\n", buffer); // Print the server's response
    }

    return 0; // Exit the program successfully
}
