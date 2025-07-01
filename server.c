#include <winsock2.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <ctype.h> //tolower()

#define DEFAULT_PORT 8080
#define INPUT_BUFSIZE 512



//apply tolower() to an entire string
char * strtolower (const char * src, char * dest) {
    char * original_dest = dest; //saving the pointer since we're incrementing it
    while (*src != '\0') {
        *dest = tolower( *src );
        src++;
        dest++;
    }
    *dest = '\0';

    return original_dest;
}

//taken from stackoverflow
char * trim_white_space(char *str)
{
    char *end;
    //Trim leading space
    while(isspace((unsigned char)*str)) 
        str++;
    if(*str == 0)  // All spaces?
            return str;
    
    //Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    // Write new null terminator character
    end[1] = '\0';
        
    return str;
}


int main(){
    WSADATA wsaData;

    int iResult;
    const char * cmdQuit = "quit";
    const char * cmdExit = "exit";
    const char * cmdHelp = "help";
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
    
    
    //display welcome banner
    char * welcome_message = "You are in a cave, everything here echoes...\n";
    int iSendResult = send(client_socket, welcome_message, strlen(welcome_message), 0);
    if (iSendResult == SOCKET_ERROR) {
        fprintf(stderr, "Failed to send welcome banner (error: %d)\n", WSAGetLastError());
        goto cleanup;
    }
    
    //enter listening loop
    
    do {

        //receive data from the client
        char recvbuf[INPUT_BUFSIZE];
        memset(recvbuf, 0, sizeof(recvbuf)); //clear buffer
        iResult = recv(client_socket, recvbuf, sizeof(recvbuf), 0);
        
        trim_white_space(recvbuf);
        char lowered_buf[INPUT_BUFSIZE];
        strtolower(recvbuf, lowered_buf); 

        int len = strlen(recvbuf);
                       
        //check for socket error
        if (iResult == SOCKET_ERROR) {
            fprintf(stderr, "Fatal error: recv() failed with SOCKET_ERROR: %d\n", WSAGetLastError());
            goto cleanup;
        } else if (iResult == 0){ 
            printf("Client closed connection. Closing sockets..");
            goto exit_success;
        
        //if data was recieved...
        } if (iResult > 0) {

            //recv() returns the number of bytes recieved if successful, (stored in iResult)
            printf("Recieved string: \"%.*s\" from the client.\n", iResult, recvbuf);
         
            //check if user wants to quit
            if ((strncmp(recvbuf, cmdQuit, len) == 0) || (strncmp(recvbuf, cmdExit, len) == 0) ) {
                printf("%s", recvbuf);
                printf("Client requested connection closed.");
                goto exit_success;
            }
            //check if user asks for help    
            else if (strncmp(recvbuf, cmdHelp, len) == 0){
                iSendResult = send(client_socket, help_options, strlen(help_options), 0);
                continue;
            
                if(iSendResult == SOCKET_ERROR){
                    printf("Failed to send data to client. Error: %d", WSAGetLastError());
                    goto cleanup;
                }
            }

        }
        //echo back to the client his string
        iSendResult = send(client_socket, recvbuf, iResult, 0);
        if(iSendResult == SOCKET_ERROR){
            printf("Failed to send data to client. Error: %d", WSAGetLastError());
            goto cleanup;
        }
    }    
    while (iResult > 0);
cleanup:
    if (client_socket != INVALID_SOCKET) {
        closesocket(client_socket);
    }
    if (server_socket != INVALID_SOCKET) {
        closesocket(server_socket);
    }
    WSACleanup();
    return EXIT_FAILURE;
exit_success:
    if (client_socket != INVALID_SOCKET) {
        closesocket(client_socket);
    }
    if (server_socket != INVALID_SOCKET) {
        closesocket(server_socket);
    }
    WSACleanup();
    return EXIT_SUCCESS;
}



