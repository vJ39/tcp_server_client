#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
int main(int argc, char *argv[]) {
    int sockfd, len, result;
    fd_set ready;
    struct timeval to;
    struct sockaddr_in address;
    char buf[1024*1024] = {0};

    struct in_addr addr;
    inet_aton("192.168.1.106", &addr);
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    address.sin_family = AF_INET;
    address.sin_addr = addr;
    address.sin_port = htons(9000);
    len = sizeof(address);

    if((result = connect(sockfd, (struct sockaddr *)&address, len)) == -1) {
        perror("connect");
        return -1;
    }
    // if( fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1) perror("fcntl");

    int i;
    int size = 0;
    FD_ZERO(&ready);
    FD_SET(sockfd, &ready);
    to.tv_sec = 1;
    to.tv_usec = 0;
    for(i = 1; i < argc; i++){
        memcpy(buf + size, argv[i], strlen(argv[i]));
        size += strlen(argv[i]);
        size ++;
    }

    write(sockfd, buf, size);
    bzero(buf, sizeof(buf));

    if(select(sockfd + 1, &ready, (fd_set *)0, (fd_set *)0, &to) == -1) {
        perror("select");
        return -1;
    }
    do{
        if(FD_ISSET(sockfd, &ready)) {
            i = read(sockfd, buf, sizeof(buf));
            write(1, buf, i);
        }
    } while(i > 0);
    close(sockfd);
    return 0;
}
