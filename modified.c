#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 30000

void serve_static_file(int new_socket, const char *filename) {
    if (strcmp(filename, "") == 0) {
        // Request for the root path ("/"), serve index.html
        filename = "index.html";
    }

    char filepath[100];
    snprintf(filepath, sizeof(filepath), "static/%s", filename);

    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        // Handle file not found error
        const char *not_found_response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\nContent-Length: 13\n\nFile not found";
        write(new_socket, not_found_response, strlen(not_found_response));
        return;
    }

    char buffer[MAX_BUFFER_SIZE];
    ssize_t read_bytes;

    // Read and send the file contents
    while ((read_bytes = read(fd, buffer, sizeof(buffer))) > 0) {
        write(new_socket, buffer, read_bytes);
    }

    // Close the file descriptor
    close(fd);
}

void handle_request(int new_socket) {
    // Read the incoming request
    char buffer[MAX_BUFFER_SIZE] = {0};
    read(new_socket, buffer, sizeof(buffer));

    // Extract the requested file from the request
    char filename[100];
    if (sscanf(buffer, "GET /%99s", filename) == 1) {
        // Serve the requested static file
        serve_static_file(new_socket, filename);
    } else {
        // Handle invalid or unsupported request
        const char *bad_request_response = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 12\n\nBad request";
        write(new_socket, bad_request_response, strlen(bad_request_response));
    }
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket;  // Declare new_socket
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    memset(address.sin_zero, '\0', sizeof address.sin_zero);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("In bind");
        exit(EXIT_FAILURE);
    }

    // Listen on the socket
    if (listen(server_fd, 10) < 0) {
        perror("In listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        // Handle the incoming request and serve static files
        handle_request(new_socket);

        // Close the socket for this connection
        close(new_socket);
    }

    return 0;
}
