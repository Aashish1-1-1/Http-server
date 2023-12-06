#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

std::string readHtmlFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening HTML file: " << filename << "\n";
        return "";
    }

    std::ostringstream content;
    content << file.rdbuf();
    file.close();

    return content.str();
}

int main() {
    // Create a socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket\n";
        return -1;
    }

    // Bind the socket to an address and port
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // Listen for incoming connections
    listen(serverSocket, 5);

    std::cout << "Server listening on port 8080...\n";

    while (true) {
        // Accept a client connection
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == -1) {
            std::cerr << "Error accepting connection\n";
            continue;  // Continue to the next iteration of the loop
        }

        // Read HTML content from a file
        std::string htmlContent = readHtmlFile("index.html");

        // Construct an HTTP response with the HTML content
        std::string httpResponse = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/html\r\n"
                                   "\r\n" + htmlContent;

        // Send the HTTP response to the client
        send(clientSocket, httpResponse.c_str(), httpResponse.size(), 0);

        // Close the client socket
        close(clientSocket);
    }

    // Close the server socket (This won't be reached in this example)
    close(serverSocket);

    return 0;
}
