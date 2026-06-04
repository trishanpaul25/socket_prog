#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>


int main(){
    struct sockaddr_in serv_addr, cli_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    unsigned short port = 9000;
    char ip[] = "127.0.0.1";

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_aton(ip, &serv_addr.sin_addr);

    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(serv_sock < 0){
        printf("Cannot create socket\n");
        exit(1);
    }

    int bind_st = bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if(bind_st<0){
        printf("Cannot bind socket\n");
        exit(1);
    }

    int list_st = listen(serv_sock, 10);
    if(list_st < 0){
        printf("Cannot listen\n");
        exit(1);
    }

    socklen_t cli_addr_len = sizeof(cli_addr);

    while(1){
        printf("Server listening for clients\n");

        int cli_sock = accept(serv_sock, (struct sockaddr*)&cli_addr, &cli_addr_len);
        if(cli_sock < 0){
            printf("Cannot connect to client\n");
            continue;
        }

        printf("Connection established with client %s\n",inet_ntoa(cli_addr.sin_addr));

        char buff[128];
        bzero(buff,sizeof(buff));

        int r = recv(cli_sock, buff, sizeof(buff) - 1, 0);
        if(r <= 0){
            printf("Cannot read message from client\n");
            break;
        }

        buff[r] = '\0';
        buff[strcspn(buff,"\r\n")] = '\0';

        printf("Message from client : %s\n",buff);

        int w = send(cli_sock,buff,strlen(buff),0);
        if(w <= 0){
            printf("Cannot send echoed message to client\n");
            break;
        }

        printf("Message sent to client %s\n",inet_ntoa(cli_addr.sin_addr));
        close(cli_sock);
    }
    close(serv_sock);
    return 0;
}
