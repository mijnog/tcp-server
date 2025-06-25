#include <winsock2.h>
#include <stdio.h>
#include <ws2tcpip.h>

#define DEFAULT_PORT 8080
   
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
}
