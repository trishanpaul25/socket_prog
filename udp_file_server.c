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
    struct sockaddr_in serv_addr, cli_addr;
    unsigned short port = 8000;
    char ip[] = "127.0.0.1";

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_aton(ip, &serv_addr.sin_addr);

    int serv_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serv_socket < 0) {
        printf("Cannot create socket\n");
        exit(1);
    }

    int bind_status = bind(serv_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (bind_status < 0) {
        printf("Cannot bind server\n");
        close(serv_socket);
        exit(1);
    }

    printf("[Server] UDP Command Server is running on port %d...\n", port);

    socklen_t cli_addr_len = sizeof(cli_addr);

    while (1) {
        char cmd_buff[CMD_SIZE];
        bzero(cmd_buff, sizeof(cmd_buff));
        
        int r = recvfrom(serv_socket, cmd_buff, sizeof(cmd_buff) - 1, 0, (struct sockaddr*)&cli_addr, &cli_addr_len);
        if (r <= 0) {
            printf("[Server] Error reading data.\n");
            continue;
        }
        
        cmd_buff[strcspn(cmd_buff, "\r\n")] = '\0';

        printf("\n[Server] Received datagram from client: %s\n", inet_ntoa(cli_addr.sin_addr));

        if (strcmp(cmd_buff, "exit") == 0) {
            printf("[Server] Client notified termination ('exit').\n");

            sendto(serv_socket, "Session closed safely.\n", 23, 0, (struct sockaddr*)&cli_addr, cli_addr_len);
            continue;
        }

        if (strlen(cmd_buff) == 0) {
            sendto(serv_socket, "\n", 1, 0, (struct sockaddr*)&cli_addr, cli_addr_len);
            continue;
        }

        printf("[Server] Executing Unix Command: '%s'\n", cmd_buff);

        FILE *fp = popen(cmd_buff, "r");
        char result_buff[BUFFER_SIZE];
        bzero(result_buff, sizeof(result_buff));

        if (fp == NULL) {
            strcpy(result_buff, "Error: Failed to execute command on server.\n");
        } else {
            char path[256];
            while (fgets(path, sizeof(path), fp) != NULL) {
                if (strlen(result_buff) + strlen(path) < BUFFER_SIZE - 1) {
                    strcat(result_buff, path);
                }
            }
            pclose(fp);
            
            if (strlen(result_buff) == 0) {
                strcpy(result_buff, "Command executed successfully with no output.\n");
            }
        }

        printf("--- Command Output Begin ---\n%s----------------------------\n", result_buff);
        
        FILE *log_file = fopen("command_outputs.txt", "w");
        if (log_file != NULL) {
            fprintf(log_file, "Command: %s\n", cmd_buff);
            fprintf(log_file, "Output:\n%s", result_buff);
            fclose(log_file);
            printf("[Server] Successfully saved latest output to 'command_outputs.txt'\n");
        } else {
            printf("[Server] Warning: Could not open log file to save output.\n");
        }
        int w = sendto(serv_socket, result_buff, strlen(result_buff), 0, (struct sockaddr*)&cli_addr, cli_addr_len);
        if (w < 0) {
            printf("Cannot send data back to client\n");
        }
    }
    
    close(serv_socket);
    return 0;
}
