#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 8080
#define MAX_FILENAME_LENGTH 256
#define BUFFER_SIZE 4096

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

int main(void)
{
    char filename[MAX_FILENAME_LENGTH];
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_address = {0};
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Could not create socket");
        return EXIT_FAILURE;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) != 1) {
        fprintf(stderr, "Invalid server address.\n");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    if (connect(socket_fd, (struct sockaddr *)&server_address,
                sizeof(server_address)) < 0) {
        perror("Could not connect to server");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    printf("Enter filename: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        fprintf(stderr, "Could not read filename.\n");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    if (strchr(filename, '\n') == NULL && !feof(stdin)) {
        int character;
        while ((character = getchar()) != '\n' && character != EOF) {
        }
        fprintf(stderr, "Filename is too long.\n");
        close(socket_fd);
        return EXIT_FAILURE;
    }
    filename[strcspn(filename, "\r\n")] = '\0';
    if (filename[0] == '\0') {
        fprintf(stderr, "Filename cannot be empty.\n");
        close(socket_fd);
        return EXIT_FAILURE;
    }
    if (strlen(filename) >= MAX_FILENAME_LENGTH - 1) {
        fprintf(stderr, "Filename is too long.\n");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    if (send_all(socket_fd, filename, strlen(filename) + 1) < 0) {
        perror("Could not send filename");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    unsigned char status;
    ssize_t status_bytes;
    do {
        status_bytes = recv(socket_fd, &status, sizeof(status), 0);
    } while (status_bytes < 0 && errno == EINTR);

    if (status_bytes != 1 || (status != 'D' && status != 'E')) {
        fprintf(stderr, "Server returned an invalid response.\n");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    FILE *output = status == 'E' ? stderr : stdout;
    ssize_t bytes_received;
    while ((bytes_received = recv(socket_fd, buffer, sizeof(buffer), 0)) > 0) {
        if (fwrite(buffer, 1, (size_t)bytes_received, output) !=
            (size_t)bytes_received) {
            perror("Could not write response");
            close(socket_fd);
            return EXIT_FAILURE;
        }
    }
    if (bytes_received < 0) {
        perror("Could not receive file contents");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    close(socket_fd);
    return status == 'D' ? EXIT_SUCCESS : EXIT_FAILURE;
}
