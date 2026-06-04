#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

void display_menu() {
    printf("\n--- Calculator Menu ---\n");
    printf("1. Binary to Decimal\n");
    printf("2. Decimal to Binary\n");
    printf("3. BODMAS Calculation (Format: num1 op num2, e.g., 5 + 3)\n");
    printf("4. Exit\n");
    printf("Enter choice: ");
}

int main(){
    struct sockaddr_in serv_addr;

    unsigned short port = 12200;
    char ip[] = "127.0.0.1";

    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_port = htons(port);
    serv_addr.sin_family = AF_INET;
    inet_aton(ip, &serv_addr.sin_addr);

    int cli_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(cli_sock < 0){
        printf("Cannot Create socket in server\n");
        exit(1);
    }

    int connect_st = connect(cli_sock, (struct sockaddr*)&serv_addr,sizeof(serv_addr));
    if(connect_st < 0){
        printf("Client cannot connect to server\n");
        close(cli_sock);
        exit(1);
    }

    printf("Connection established with the server\n");

    int choice;
    char buffer[1024];

    while(1){
        display_menu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid selection.\n");
            break;
        }
        getchar(); // clear newline character left by scanf

        // Convert choice to network byte order and send
        int net_choice = htonl(choice);
        send(cli_sock, &net_choice, sizeof(net_choice), 0);

        if(choice == 4) {
            printf("Exiting application...\n");
            break;
        }

        // Get matching input strings based on calculation choice
        bzero(buffer, sizeof(buffer));
        if (choice == 1) {
            printf("Enter binary string: ");
            fgets(buffer, sizeof(buffer), stdin);
        } else if (choice == 2) {
            printf("Enter decimal number: ");
            fgets(buffer, sizeof(buffer), stdin);
        } else if (choice == 3) {
            printf("Enter simple expression (e.g., 12 * 4): ");
            fgets(buffer, sizeof(buffer), stdin);
        } else {
            printf("Invalid option. Try again.\n");
            continue;
        }

        // Send payload data to server
        send(cli_sock, buffer, strlen(buffer), 0);

        // Receive computation back from server
        bzero(buffer, sizeof(buffer));
        recv(cli_sock, buffer, sizeof(buffer), 0);
        printf("Server Response -> %s\n", buffer);
    }

    close(cli_sock);
    return 0;
}
