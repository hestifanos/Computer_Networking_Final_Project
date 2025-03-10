// // tcp_server.c
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>

// #define PORT 8080 // Port number to connect to the server
// #define BUFFER_SIZE 101  // 100 bytes for data + 1 byte for type indicator

// void handle_client(int client_socket) {//function to handle the client request 
//     char filename[256];//to store the filename
//     memset(filename, 0, sizeof(filename));//fill the first 256 bytes of filename with 0
//     if (read(client_socket, filename, sizeof(filename)) <= 0) {//read the filename from the client socket and store in filename
//         perror("Failed to read filename");
//         close(client_socket);
//         return;
//     }
    
//     FILE *file = fopen(filename, "rb");//open file in binary mode  //what does rb do? //read binary mode
//     char buffer[BUFFER_SIZE];//buffer to store the data to be sent to the client
//     if (!file) {//if file is not found
//         buffer[0] = 'E';  // Error indicator
//         strcpy(buffer + 1, "Error: File not found\n");//copy the error message to the buffer
//         send(client_socket, buffer, strlen(buffer), 0);//send the buffer to the client socket with the length of the buffer and 0 flags to tell that no special flags are set
//     } else {
//         buffer[0] = 'D';  // Data indicator
//         size_t bytes_read;//to store the number of bytes read from the file
//         while ((bytes_read = fread(buffer + 1, 1, 100, file)) > 0) {//what does each parameter do?  //read 100 bytes from file and store in buffer starting from index 1   
//             send(client_socket, buffer, bytes_read + 1, 0);// send the buffer to client socket with bytes_read+1 bytes and 0 flags to tell that no special flags are set
//         }
//         fclose(file);
//     }
//     close(client_socket);
// }

// int main() {
//     int server_fd, client_socket;//server file descriptor and client socket
//     struct sockaddr_in address;//structure to store the client address
//     socklen_t addr_len = sizeof(address);//length of the address

//     server_fd = socket(AF_INET, SOCK_STREAM, 0);
//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = INADDR_ANY;//accept connections from any IP address
//     address.sin_port = htons(PORT);

//     bind(server_fd, (struct sockaddr *)&address, sizeof(address));//bind the server file descriptor to the address and the length of the address
//     listen(server_fd, 5);//listen for incoming connections with a maximum of 5 connections in the queue

//     while (1) {
//         client_socket = accept(server_fd, (struct sockaddr *)&address, &addr_len);//accept the client connection and store the client socket
//         if (client_socket >= 0) {
//             handle_client(client_socket);//handle the client request
//         }
//     }
//     close(server_fd);
//     return 0;
// }



// tcp_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 8080 // Port number to connect to the server
#define BUFFER_SIZE 101  // 100 bytes for data + 1 byte for type indicator

void handle_client(int client_socket) {
    char filename[256];
    memset(filename, 0, sizeof(filename));
    if (read(client_socket, filename, sizeof(filename)) <= 0) {
        perror("Failed to read filename");
        close(client_socket);
        return;
    }

    printf("Requested filename: %s\n", filename); // Debug print

    FILE *file = fopen(filename, "rb");
    char buffer[BUFFER_SIZE];
    if (!file) {
        buffer[0] = 'E';  // Error indicator
        strcpy(buffer + 1, "Error: File not found\n");
        send(client_socket, buffer, strlen(buffer), 0);
        perror("Error opening file"); // Debug print
    } else {
        buffer[0] = 'D';  // Data indicator
        size_t bytes_read;
        while ((bytes_read = fread(buffer + 1, 1, 100, file)) > 0) {
            send(client_socket, buffer, bytes_read + 1, 0);
        }
        fclose(file);
    }
    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, &addr_len);
        if (client_socket >= 0) {
            handle_client(client_socket);
        } else {
            perror("Accept failed");
        }
    }
    close(server_fd);
    return 0;
}
