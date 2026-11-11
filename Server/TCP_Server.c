#if defined(__APPLE__)
#define _DARWIN_C_SOURCE
#endif
#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define SERVER_PORT 8080
#define BACKLOG 5
#define MAX_FILENAME_LENGTH 256
#define BUFFER_SIZE 4096
#define FILE_DIRECTORY "files"

static int send_all(int socket_fd, const void *data, size_t length)
{
    const unsigned char *bytes = data;
    size_t sent = 0;

    while (sent < length) {
        ssize_t result = send(socket_fd, bytes + sent, length - sent, 0);
        if (result < 0 && errno == EINTR) {
            continue;
        }
        if (result <= 0) {
            return -1;
        }
        sent += (size_t)result;
    }

    return 0;
}

static int receive_filename(int socket_fd, char filename[MAX_FILENAME_LENGTH])
{
    size_t length = 0;

    while (length < MAX_FILENAME_LENGTH - 1) {
        char character;
        ssize_t result = recv(socket_fd, &character, 1, 0);
        if (result < 0 && errno == EINTR) {
            continue;
        }
        if (result <= 0) {
            return -1;
        }
        if (character == '\0') {
            filename[length] = '\0';
            return length > 0 ? 0 : -1;
        }
        filename[length++] = character;
    }

    return -1;
}

static int is_valid_filename(const char *filename)
{
    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
        return 0;
    }

    for (const unsigned char *character = (const unsigned char *)filename;
         *character != '\0'; character++) {
        if (!isalnum(*character) && *character != '.' && *character != '_' &&
            *character != '-') {
            return 0;
        }
    }

    return 1;
}

static void send_error(int socket_fd, const char *message)
{
    const unsigned char status = 'E';
    if (send_all(socket_fd, &status, sizeof(status)) < 0 ||
        send_all(socket_fd, message, strlen(message)) < 0) {
        perror("Could not send error response");
    }
}

static void handle_client(int client_fd)
{
    char filename[MAX_FILENAME_LENGTH];
    if (receive_filename(client_fd, filename) < 0 ||
        !is_valid_filename(filename)) {
        send_error(client_fd, "Invalid filename.\n");
        close(client_fd);
        return;
    }

    int directory_fd = open(FILE_DIRECTORY, O_RDONLY | O_DIRECTORY);
    if (directory_fd < 0) {
        perror("Could not open file directory");
        send_error(client_fd, "Server file directory is unavailable.\n");
        close(client_fd);
        return;
    }

    int file_fd = openat(directory_fd, filename, O_RDONLY | O_NOFOLLOW);
    close(directory_fd);
    if (file_fd < 0) {
        send_error(client_fd, "Requested file is unavailable.\n");
        close(client_fd);
        return;
    }

    struct stat file_info;
    if (fstat(file_fd, &file_info) < 0 || !S_ISREG(file_info.st_mode)) {
        close(file_fd);
        send_error(client_fd, "Requested file is unavailable.\n");
        close(client_fd);
        return;
    }

    FILE *file = fdopen(file_fd, "rb");
    if (file == NULL) {
        perror("Could not open requested file stream");
        close(file_fd);
        send_error(client_fd, "Requested file is unavailable.\n");
        close(client_fd);
        return;
    }

    const unsigned char status = 'D';
    if (send_all(client_fd, &status, sizeof(status)) < 0) {
        perror("Could not send response status");
    } else {
        char buffer[BUFFER_SIZE];
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            if (send_all(client_fd, buffer, bytes_read) < 0) {
                perror("Could not send file contents");
                break;
            }
        }
        if (ferror(file)) {
            perror("Could not read requested file");
        }
    }

    fclose(file);
    close(client_fd);
}

int main(void)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Could not create server socket");
        return EXIT_FAILURE;
    }

    int reuse_address = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_address,
                   sizeof(reuse_address)) < 0) {
        perror("Could not configure server socket");
        close(server_fd);
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);

    if (bind(server_fd, (struct sockaddr *)&server_address,
             sizeof(server_address)) < 0) {
        perror("Could not bind server socket");
        close(server_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, BACKLOG) < 0) {
        perror("Could not listen on server socket");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("Server listening on port %d\n", SERVER_PORT);
    for (;;) {
        struct sockaddr_in client_address;
        socklen_t address_length = sizeof(client_address);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_address,
                               &address_length);
        if (client_fd < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Could not accept client connection");
            continue;
        }

        handle_client(client_fd);
    }
}
