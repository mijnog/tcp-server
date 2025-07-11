#include <winsock2.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <ctype.h>
#include "lib.h"

#define DEFAULT_PORT 8080
#define INPUT_BUFSIZE 512


int main(){
    WSADATA wsaData;

    int iResult;

    const char * help_options = "type \'exit\' or \'quit\' to quit\n";
    //initialize winsock
    printf("Initializing winsock...\n");
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    /*request Winsock version 2.2, store status code in iResult*/
    
    if (iResult != 0){
        fprintf(stderr, "WSAStartup failed with error: %d\n", iResult);
        WSACleanup();
        return EXIT_FAILURE;
    }
    printf("Winsock initialized successfully!\n");
    
    //create a socket
    printf("Creating socket...\n");
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET){
        fprintf(stderr, "Fatal error: socket() failed with error: %d\n", WSAGetLastError());
        goto cleanup;
    }
    printf("Socket created successfully!\n");

    //bind socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(DEFAULT_PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    printf("Binding socket...\n");
    iResult = bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
    if (iResult == SOCKET_ERROR){
        fprintf(stderr, "Fatal error: bind() failed with error: %d\n", WSAGetLastError());
        goto cleanup;
    }
    printf("Bind successful!\n");
    
    //start listening
    printf("Starting listener...");
    iResult = listen(server_socket, SOMAXCONN);
    if (iResult == SOCKET_ERROR){
        printf("listen function failed with error: %d\n", WSAGetLastError());
        goto cleanup;
    }
    printf("Listening on port %d...\n", DEFAULT_PORT);

    //create client socket
    struct sockaddr_in client_address;
    int client_address_len = sizeof(client_address);
    SOCKET client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len);
    if (client_socket == INVALID_SOCKET){
        fprintf(stderr, "Error creating client socket: %d\n", WSAGetLastError());
        goto cleanup;
    }

    

    // Enter command loop
    char inputbuf[INPUT_BUFSIZE];
    char outputbuf[1024];
    while (1) {
        printf("Type a PowerShell command: ");
        fflush(stdout);

        // Read user input from stdin
        if (!fgets(inputbuf, sizeof(inputbuf), stdin)) {
            printf("Input error or EOF. Exiting.\n");
            break;
        }

        // Remove trailing newline
        size_t len = strlen(inputbuf);
        if (len > 0 && inputbuf[len - 1] == '\n') {
            inputbuf[len - 1] = '\0';
            len--;
        }

        // Trim whitespace
        char *trimmed = trim_white_space(inputbuf);

        // If empty, prompt again
        if (strlen(trimmed) == 0) {
            continue;
        }

        // Lowercase for command checks
        char lowered_buf[INPUT_BUFSIZE];
        strtolower(trimmed, lowered_buf);

        // Check for quit/exit/help
        if ((strcmp(lowered_buf, "quit") == 0) || (strcmp(lowered_buf, "exit") == 0)) {
            printf("Exiting server command loop.\n");
            break;
        } else if (strcmp(lowered_buf, "help") == 0) {
            printf("%s", help_options);
            continue;
        }

        // Send command to client
        int iSendResult = send(client_socket, trimmed, (int)strlen(trimmed), 0);
        if (iSendResult == SOCKET_ERROR) {
            printf("Failed to send data to client. Error: %d\n", WSAGetLastError());
            goto cleanup;
        }

        // Receive and print output from client until EOT (\x04) or connection closed
        printf("Output from client:\n");
        int done = 0;
        while (!done) {
            int bytes = recv(client_socket, outputbuf, sizeof(outputbuf) - 1, 0);
            if (bytes <= 0) {
                printf("Client disconnected or error occurred.\n");
                goto cleanup;
            }
            // Check for EOT marker
            for (int i = 0; i < bytes; ++i) {
                if ((unsigned char)outputbuf[i] == 0x04) {
                    if (i > 0) {
                        fwrite(outputbuf, 1, i, stdout);
                        fflush(stdout);
                    }
                    done = 1;
                    break;
                }
            }
            if (!done) {
                fwrite(outputbuf, 1, bytes, stdout);
            }
        }
        printf("\n");
    }
cleanup:
    if (client_socket != INVALID_SOCKET) {
        closesocket(client_socket);
    }
    if (server_socket != INVALID_SOCKET) {
        closesocket(server_socket);
    }
    WSACleanup();
    return EXIT_FAILURE;

}



