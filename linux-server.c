#include <stdio.h>      // Standard I/O functions (printf, perror)
#include <stdlib.h>     // Standard library functions (exit)
#include <string.h>     // String manipulation functions (memset)
#include <unistd.h>     // POSIX operating system API (close, read, write)

#include <sys/socket.h> // Core socket functions and data structures
#include <netinet/in.h> // Internet address structures (sockaddr_in, INADDR_ANY)
#include <arpa/inet.h>  // Functions for Internet address manipulation (inet_addr, inet_pton)
#include <errno.h>

#define DEFAULT_PORT 8080
#define BACKLOG 50
#define MAX_MESSAGE_SIZE 512
int main(){
	
int fd_server_socket;
int iResult;

	//create server socket
	printf("Creating server socket...\n");
	fd_server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == fd_server_socket){ 
		printf("Error creating server socket: %d\n", errno);
		return 1;
	} else 
		printf("Successfuly created server socket!\n");

	//bind server socket
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(DEFAULT_PORT); //change to network (big-endian) byte order
	server_address.sin_addr.s_addr = INADDR_ANY;      
    
	printf("Binding server socket to port %d...\n", DEFAULT_PORT);
	iResult = bind(fd_server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
	if (iResult == -1){
		printf("Error binding server socket to port %d: %d\n", DEFAULT_PORT, errno);
		return -1;
	} else
		printf("Server socket successfully bound to port %d!\n", DEFAULT_PORT);
	
	//listen on DEFAULT_PORT for incoming connections
	
	if (listen(fd_server_socket, BACKLOG == -1)){
		printf("Listen failed with error: %d\n", errno);
		return 1;
	}
	printf("Listening on port %d...\n", DEFAULT_PORT);

	//create client_socket
	struct sockaddr client_address;
	socklen_t client_address_len = sizeof(client_address);
	int fd_client_socket = accept(fd_server_socket, (struct sockaddr*) &client_address, &client_address_len);
	printf("Creating client socket...\n");
	if (fd_client_socket == -1){
		printf("Error creating client socket: %d\n", errno);
	}
	printf("Client socket successfully created!\n");

	//send	

	char recvbuf[MAX_MESSAGE_SIZE];

	read(fd_client_socket, recvbuf, sizeof(recvbuf));
	printf("Client sent \'%s\'", recvbuf);
	
	char status = 0;
  	// https://man7.org/linux/man-pages/man2/write.2.html
  	// write(fd, buf, count) writes up to count bytes from the buffer starting at buf to the file referred to by the file descriptor fd
  	// In this case we simply send a single byte 0 to indicate we have received the message
  	write(fd_client_socket, &status, 1);

  	// https://man7.org/linux/man-pages/man2/close.2.html
  	// close() closes a file descriptor, so that it no longer refers to any file and may be reused
  	close(fd_server_socket);

  	return 0;
}


