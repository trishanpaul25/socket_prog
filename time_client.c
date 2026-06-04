#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>


int main(){
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    unsigned short port = 9500;
    char ip[] = "127.0.0.1";

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_aton(ip, &serv_addr.sin_addr);

    int cli_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(cli_sock < 0){
        printf("Cannot create socket\n");
        exit(1);
    }

    int conn_st = connect(cli_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if(conn_st < 0){
        printf("Cannot connect to server\n");
        exit(1);
    }

    printf("Client Connected to server\n");

    while(1){
        char buff[128];
        bzero(buff,sizeof(buff));

        printf("Input : ");
        fgets(buff,sizeof(buff)-1,stdin);

        buff[strcspn(buff,"\r\n")] = '\0';

        int w = send(cli_sock,buff,strlen(buff),0);
        if(w <= 0){
            printf("Cannot send messsage to server\n");
            break;
        }

        if(strcmp(buff,"exit") == 0){
            printf("Client closing connection\n");
            break;
        }

        bzero(buff,sizeof(buff));

        int r = recv(cli_sock,buff,sizeof(buff)-1,0);
        if(r <= 0){
            printf("Cannot recv message from server\n");
            break;
        }

        buff[r] = '\0';
        printf("Time : %s\n",buff);
    }
    close(cli_sock);
    return 0;
}
