#include <winsock2.h>
#include <stdio.h>
#include <ws2tcpip.h>

#define DEFAULT_PORT 8080



char *trimwhitespace(char *str)
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
   
    //initialize winsock
    printf("Initializing winsock...\n");
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    /*request Winsock version 2.2, store status code in iResult*/
    
    if (iResult != 0){
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    printf("Winsock initialized successfully!\n");
    
    //create a socket
    printf("Creating socket...\n");
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET){
        printf("Fatal error: socket() failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
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
        printf("Fatal error: bind() failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("Bind successful!\n");
    
    //start listening
    printf("Starting listener...");
    iResult = listen(server_socket, SOMAXCONN);
    if (iResult == SOCKET_ERROR){
        printf("listen function failed with error: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    printf("Listening on port %d...\n", DEFAULT_PORT);

    //create client socket
    struct sockaddr_in client_address;
    int client_address_len = sizeof(client_address);
    SOCKET client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len);
    if (client_socket == INVALID_SOCKET){
        printf("Error creating client socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    
    
    //send welcome banner
    char * welcome_message = "You are in a cave, everything here echoes...";
    int iSendResult = send(client_socket, welcome_message, strlen(welcome_message), 0);
    if (iSendResult == SOCKET_ERROR) {
        printf("Failed to send welcome banner (error: %d)\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }
    
    //enter listening loop
    
    do {

        //receive data from the client
        char recvbuf[512];
        memset(recvbuf, 0, sizeof(recvbuf);
        iResult = recv(client_socket, recvbuf, sizeof(recvbuf), 0);
        
        if (strncmp(recvbuf, "quit", iResult) == 0){
            printf("Client requested connection closed.");
            closesocket(server_socket);
            printf("Server socket closed");
            closesocket(client_socket);
            printf("Client socket closed");
            WSACleanup();
            return 0;
        }
        else if (iResult == SOCKET_ERROR) {
            printf("Fatal error: recv() failed with SOCKET_ERROR: %d\n", WSAGetLastError());
            closesocket(client_socket);
            printf("Client socket closed");
            closesocket(server_socket);
            printf("Server socket closed");
            WSACleanup();
            return 1;
        } else if (iResult == 0){ 
            printf("Client closed connection. Closing sockets..");
            closesocket(client_socket);
            printf("Client socket closed");
            closesocket(server_socket);
            printf("Server socket closed");
            return 0;
        } else if (iResult > 0) {

            //recv() returns the number of bytes recieved if successful, (stored in iResult)
            printf("Recieved: %.*s\n", iResult, recvbuf);
        
            //echo back to the client his string
            iSendResult = send(client_socket, recvbuf, iResult, 0);
            if(iSendResult == SOCKET_ERROR){
                printf("Failed to send data to client. Error: %d", WSAGetLastError());
                closesocket(client_socket);
                WSACleanup();
                return 1;
            }
        }
        else {
            printf("Debug: recv returns: %d. INVESTIGATE THIS", iResult);
            closesocket(client_socket);
            WSACleanup();
            return 1;
        }
    } 
    while (iResult > 0);

}

