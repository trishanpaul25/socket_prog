#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<time.h>

int main(){
    struct sockaddr_in serv_addr, cli_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    unsigned short port = 9500;
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

        while(1){
            char buff[128];
            bzero(buff,sizeof(buff));

            int r = recv(cli_sock,buff,sizeof(buff)-1,0);
            if(r <= 0){
                printf("Cannot recv message from client\n");
                break;
            }

            buff[r] = '\0';
            buff[strcspn(buff,"\r\n")] = '\0';

            if(strcmp(buff,"exit") == 0){
                printf("Client closed connection\n");
                break;
            }

            int a,b,res;
            char op;
            char result[128];
            bzero(result,sizeof(res));

            sscanf(buff,"%d %c %d",&a,&op,&b);

            switch(op){
                case '+' : res = a+b;
                sprintf(result,"result : %d",res);
                break;
                case '-' : res = a-b;
                sprintf(result,"result : %d",res);
                break;
                case '*' : res = a*b;
                sprintf(result,"result : %d",res);
                break;
                case '/' :if(b == 0) sprintf(result,"Cannot divide by zero");
                else{
                    res = a/b;
                    sprintf(result,"result : %d",res);
                }
                break;
                case '%' : if(b == 0) sprintf(result,"Cannot modulo by zero");
                else{
                    res = a%b;
                    sprintf(result,"result : %d",res);
                }
                break;
            }

            int w = send(cli_sock,result,strlen(result),0);
            if(w <= 0){
                printf("Cannot send result to client\n");
                break;
            }

            printf("%s\n",result);
        }
        close(cli_sock);
    }
    close(serv_sock);
    return 0;
}
