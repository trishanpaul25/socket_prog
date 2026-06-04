#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<math.h>

int solve(char s[]){

    while(s[0] == '(' && s[strlen(s) - 1] == ')'){
        int bracket_count = 0, balance = 1;
        for(int i = 1; i < strlen(s) - 1; i++){
            if(s[i] == '(') bracket_count++;
            if(s[i] == ')') bracket_count--;
            if(bracket_count < 0) balance = 0;
        }

        if(balance){
            s[strlen(s) - 1] = '\0';
            s++;
        }
        else break;
    }

    int bracket_count = 0, target_pos = -1;
    char target_op = '\0';

    char pri[] = {'+','-','*','/'};

    for(int p = 0; p < 4; p++){
        for(int i = strlen(s) - 1; i >= 0; i--){
            if(s[i] == ')') bracket_count++;
            else if(s[i] == '(') bracket_count--;
            else if(bracket_count == 0 && s[i] == pri[p]){
                target_op = s[i];
                target_pos = i;
                break;
            }
        }
        if(target_op) break;
    }

    if(target_op){
        s[target_pos] = '\0';
        int left = solve(s);
        int right = solve(&s[target_pos+1]);
        if(target_op == '+') return left+right;
        if(target_op == '-') return left-right;
        if(target_op == '*') return left*right;
        if(target_op == '/') if(right!=0) return left/right;
    }

    return atof(s);
}

int main(){
    struct sockaddr_in serv_addr, cli_addr;
    unsigned short port = 12200;
    char ip[] = "127.0.0.1";

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_port = htons(port);
    serv_addr.sin_family = AF_INET;
    inet_aton(ip, &serv_addr.sin_addr);

    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(serv_sock < 0) { printf("Cannot Create socket\n"); exit(1); }

    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Cannot Bind socket\n"); close(serv_sock); exit(1);
    }

    listen(serv_sock, 10);
    socklen_t cli_addr_len = sizeof(cli_addr);

    while(1){
        printf("\nServer listening...\n");
        int cli_sock = accept(serv_sock, (struct sockaddr*)&cli_addr, &cli_addr_len);
        if(cli_sock < 0) break;
        printf("Client connected: %s\n", inet_ntoa(cli_addr.sin_addr));

        int choice;
        char buffer[1024], response[1024];

        while(1){
            if (recv(cli_sock, &choice, sizeof(choice), 0) <= 0) break;
            choice = ntohl(choice);
            if (choice == 4) break;

            bzero(buffer, sizeof(buffer));
            recv(cli_sock, buffer, sizeof(buffer), 0);
            buffer[strcspn(buffer, "\n\r ")] = 0; // Strip newlines and spaces

            bzero(response, sizeof(response));

            if (choice == 1) { // Binary to Decimal
                long bin = atol(buffer);
                int dec = 0, i = 0;
                while (bin != 0) {
                    dec += (bin % 10) * pow(2, i++);
                    bin /= 10;
                }
                sprintf(response, "Decimal: %d", dec);
            } 
            else if (choice == 2) { // Decimal to Binary
                int dec = atoi(buffer);
                char temp[1024] = "";
                if (dec == 0) strcpy(response, "Binary: 0");
                else {
                    while (dec > 0) {
                        sprintf(temp + strlen(temp), "%d", dec % 2);
                        dec /= 2;
                    }
                    // Reverse the string cleanly
                    int len = strlen(temp);
                    char reversed[1024] = "";
                    for(int j=0; j<len; j++) reversed[j] = temp[len-1-j];
                    sprintf(response, "Binary: %s", reversed);
                }
            } 
            else if (choice == 3) { // BODMAS with Brackets
                int result = solve(buffer);
                sprintf(response, "BODMAS result: %d", result);
            }

            // Print output on Server Terminal
            printf("[Choice %d] Input: %s -> %s\n", choice, buffer, response);
            // Send output to Client Terminal
            send(cli_sock, response, strlen(response), 0);
        }
        printf("Client disconnected.\n");
        close(cli_sock);
    }
    close(serv_sock);
    return 0;
}
