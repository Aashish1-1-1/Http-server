#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

char* readHtmlFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening HTML file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* content = (char*)malloc(fileSize + 1);
    if (!content) {
        fprintf(stderr, "Memory allocation error\n");
        fclose(file);
        return NULL;
    }

    fread(content, 1, fileSize, file);
    content[fileSize] = '\0';

    fclose(file);

    return content;
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error creating socket");
        return -1;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // Listen for incoming connections
    listen(serverSocket, 5);

    printf("Server listening on port 8080...\n");

    while (1) {
        // Accept a client connection
        int clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == -1) {
            perror("Error accepting connection");
            continue;  // Continue to the next iteration of the loop
        }

        // Read HTML content from a file
        char* htmlContent = readHtmlFile("./static/index.html");
        if (!htmlContent) {
            close(clientSocket);
            continue;          }

        // Construct an HTTP response with the HTML content
        char httpResponse[BUFFER_SIZE];
        snprintf(httpResponse, BUFFER_SIZE, "HTTP/1.1 200 OK\r\n"
                                            "Content-Type: text/html\r\n"
                                            "\r\n%s", htmlContent);

        // Send the HTTP response to the client
        send(clientSocket, httpResponse, strlen(httpResponse), 0);

        close(clientSocket);

        free(htmlContent);
    }

    close(serverSocket);

    return 0;
}
