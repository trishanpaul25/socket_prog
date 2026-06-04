#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

void display(){
    printf("1. Binary to Decimal\n");
    printf("2. Decimal to binary\n");
    printf("3. Bodmas Calculation\n");
    printf("4. Exit\n");
    printf("Enter choice : ");
}
int main(){
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    unsigned short port = 9000;
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
        int ch;
        display();
        scanf("%d",&ch);
        getchar();

        if(ch != 1 && ch != 2 && ch != 3 && ch != 4){
            printf("Invalid choice\n");
            continue;
        }
        
        int net_ch = htonl(ch);

        send(cli_sock,&net_ch,sizeof(net_ch),0);

        if(ch == 4){
            printf("Exiting...\n");
            break;
        }

        bzero(buff,sizeof(buff));
        printf("Input : ");
        fgets(buff,sizeof(buff)-1,stdin);

        buff[strcspn(buff,"\r\n")] = '\0';
        send(cli_sock,buff,strlen(buff),0);


        bzero(buff,sizeof(buff));
        recv(cli_sock,buff,sizeof(buff)-1,0);


        printf("%s\n",buff);
    }
    close(cli_sock);
    return 0;
}
