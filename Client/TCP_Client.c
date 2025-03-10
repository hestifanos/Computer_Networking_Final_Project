// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>//for memset function that fills the first n bytes of the memory area pointed to by s with the constant byte c 
// #include <unistd.h>//for close function to close the socket connection 
// #include <arpa/inet.h>//for inet_pton function to convert IP address from text to binary form

// #define PORT 8080 //port number to connect to the server 
// #define BUFFER_SIZE 101 // 100 bytes for data + 1 byte for type indicator

// int main() {
//     int sock = socket(AF_INET, SOCK_STREAM, 0);//create a socket with IPv4, TCP and default protocol
//     struct sockaddr_in server_address;//structure to store the server address

//     server_address.sin_family = AF_INET; //IPv4
//     server_address.sin_port = htons(PORT);//convert port number to network byte order
//     inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);//convert IP address from text to binary form

//     if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {//connect to the server with the socket and server address
//         perror("Connection failed");
//         return 1;
//     }

//     char filename[256];//to store the filename
//     printf("Enter filename: ");//ask the user to enter the filename
//     scanf("%s", filename);
//     send(sock, filename, strlen(filename) + 1, 0);//send the filename to the server with the socket and the length of the filename + 1 byte for null terminator

//     char buffer[BUFFER_SIZE];//buffer to store the data received from the server 
//     FILE *file = NULL;//file pointer to store the file to be saved
//     ssize_t bytes_received;//to store the number of bytes received from the server

//     // Define the save path explicitly
//     char save_path[300];//to store the path where the file will be saved
//     snprintf(save_path, sizeof(save_path), "./%s", filename);  // Save in current directory. s is the filename and . is the current directory snprintf is used to write formatted output to a string

//     while ((bytes_received = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {//receive the data from the server and store in buffer
//         if (buffer[0] == 'E') {
//             printf("%s", buffer + 1);  // Display error message
//             break;
//         }
//         if (!file) {
//             file = fopen(save_path, "wb");  // Open the file explicitly in the current directory
//             if (!file) {
//                 perror("Error opening file for writing");
//                 break;
//             }
//         }
//         fwrite(buffer + 1, 1, bytes_received - 1, file);//write the data from buffer to the file
//     }

//     if (file) {
//         fclose(file);
//         printf("File received successfully and saved as: %s\n", save_path);
//     }

//     close(sock);
//     return 0;
// }


// tcp_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 101

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char filename[256];
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // Get the filename from the user
    printf("Enter filename: ");
    scanf("%s", filename);

    // Send the filename to the server
    send(sock, filename, strlen(filename), 0);

    // Read the response from the server
    int bytes_read;
    while ((bytes_read = read(sock, buffer, BUFFER_SIZE)) > 0) {
        if (buffer[0] == 'E') {
            printf("%s", buffer + 1); // Print error message
            break;
        } else if (buffer[0] == 'D') {
            printf("%s", buffer + 1); // Print file contents
        }
    }

    close(sock);
    return 0;
}