#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

int main() {
    struct sockaddr_in serv_addr, cli_addr;
    unsigned short port = 9000;
    char ip[] = "127.0.0.1";

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_aton(ip, &serv_addr.sin_addr);

    int serv_socket = socket(AF_INET, SOCK_STREAM, 0);
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

    int listen_status = listen(serv_socket, 5);
    if (listen_status < 0) {
        printf("Server cannot listen to clients\n");
        close(serv_socket);
        exit(1);
    }

    socklen_t cli_addr_len = sizeof(cli_addr);
    
    FILE *log_file = fopen("command_outputs.txt", "a");
    if (log_file == NULL) {
        printf("Failed to open or create output log file.\n");
        close(serv_socket);
        exit(1);
    }

    while (1) {
        printf("\n[Server] Waiting for a client to connect...\n");

        int cli_socket = accept(serv_socket, (struct sockaddr*)&cli_addr, &cli_addr_len);
        if (cli_socket < 0) {
            printf("Cannot accept clients\n");
            continue; 
        }
        printf("[Server] Connected to client: %s\n", inet_ntoa(cli_addr.sin_addr));

        // Infinite loop for the CURRENT active client session
        while (1) {
            char cmd_buff[256];
            bzero(cmd_buff, sizeof(cmd_buff));
            
            int r = recv(cli_socket, cmd_buff, sizeof(cmd_buff) - 1, 0);
            if (r <= 0) {
                printf("[Server] Client disconnected or error reading data.\n");
                break;
            }
            
            // Remove trailing newlines and carriage returns
            cmd_buff[strcspn(cmd_buff, "\r\n")] = '\0';

            // Check if the client requested an exit session
            if (strcmp(cmd_buff, "exit") == 0) {
                printf("[Server] Client requested termination ('exit'). Closing session.\n");
                break;
            }

            // Skip execution if client accidentally sends an empty string
            if (strlen(cmd_buff) == 0) {
                send(cli_socket, "\n", 1, 0);
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

            // Print output locally on server side
            printf("--- Command Output Begin ---\n%s----------------------------\n", result_buff);

            // Log command and output to text track file
            fprintf(log_file, "Command: %s\n", cmd_buff);
            fprintf(log_file, "Output:\n%s", result_buff);
            fprintf(log_file, "--------------------------------------------------------\n");
            fflush(log_file); 

            // Send full output block back to client
            int w = send(cli_socket, result_buff, strlen(result_buff), 0);
            if (w < 0) {
                printf("Cannot send data back to client\n");
                break;
            }
        }
        close(cli_socket);
        printf("[Server] Session ended. Cleaning up resources.\n");
    }
    
    fclose(log_file);
    close(serv_socket);
    return 0;
}
