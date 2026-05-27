#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define CMD_SIZE 512

int main() {
    unsigned short port = 8000;
    char ip[] = "127.0.0.1";

    struct sockaddr_in serv_addr_config;
    bzero(&serv_addr_config, sizeof(serv_addr_config));
    serv_addr_config.sin_family = AF_INET;
    serv_addr_config.sin_port = htons(port);
    inet_aton(ip, &serv_addr_config.sin_addr);

    int cli_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_socket < 0) {
        printf("Cannot create socket\n");
        exit(1);
    }

    int connect_status = connect(cli_socket, (struct sockaddr*)&serv_addr_config, sizeof(serv_addr_config));
    if (connect_status < 0) {
        printf("Cannot connect to server\n");
        close(cli_socket);
        exit(1);
    }

    printf("Successfully connected to the Remote Shell Server.\n");
    printf("Type your Unix commands below. Type 'exit' to quit.\n\n");

    while (1) {
        // FIXED: Declared as arrays for user input strings
        char cmd_input[CMD_SIZE];
        char clean_cmd[CMD_SIZE];
        
        printf("Remote-Shell$ ");
        
        if (fgets(cmd_input, sizeof(cmd_input), stdin) == NULL) {
            printf("\nError reading input.\n");
            break;
        }

        strcpy(clean_cmd, cmd_input);
        clean_cmd[strcspn(clean_cmd, "\r\n")] = '\0';

        if (strcmp(clean_cmd, "exit") == 0) {
            send(cli_socket, "exit", 4, 0);
            printf("Exiting client interface. Goodbye!\n");
            break;
        }

        int w = send(cli_socket, cmd_input, strlen(cmd_input), 0);
        if (w < 0) {
            printf("Connection dropped. Cannot send command to server.\n");
            break;
        }

        char response_buff[BUFFER_SIZE];
        bzero(response_buff, sizeof(response_buff));
        
        int r = recv(cli_socket, response_buff, sizeof(response_buff) - 1, 0);
        if (r <= 0) {
            printf("Server closed connection or error occurred.\n");
            break;
        }
        
        printf("%s\n", response_buff);
    }

    close(cli_socket);
    return 0;
}
