#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/jail.h>
#include <sys/uio.h>
int main(int argc, char *argv[]) {
    int sockfd, len, result;
    struct sockaddr_in address;
    char buf[0xffff] = {0};

    struct in_addr addr;
    inet_aton("192.168.1.106", &addr);
    /*
    int jid;
    struct iovec iov[8];
    iov[0].iov_base = "path";
    iov[0].iov_len  = sizeof("path");
    iov[1].iov_base = "/jail/echo";
    iov[1].iov_len  = sizeof("/jail/echo");
    iov[2].iov_base = "host.hostname";
    iov[2].iov_len  = sizeof("host.hostname");
    iov[3].iov_base = "echo_client.example.jp";
    iov[3].iov_len  = sizeof("echo_client.example.jp");
    iov[4].iov_base = "name";
    iov[4].iov_len  = sizeof("name");
    iov[5].iov_base = "echo_client";
    iov[5].iov_len  = sizeof("echo_client");
    iov[6].iov_base = "ip4.addr";
    iov[6].iov_len  = sizeof("ip4.addr");
    iov[7].iov_base = &addr;
    iov[7].iov_len  = sizeof addr;
    jid = jail_set(iov, 8, JAIL_CREATE | JAIL_ATTACH);

    printf("Jail ID = %d\n", jid);
    */

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    address.sin_family = AF_INET;
    address.sin_addr = addr;
    address.sin_port = htons(9000);
    len = sizeof(address);

    if((result = connect(sockfd, (struct sockaddr *)&address, len)) == -1) {
        printf("error\n");
        return -1;
    }
    int i;
    int size = 0;
    const char sep[] = {0x0a};
    for(i = 1; i < argc; i++){
        memcpy(buf + size, argv[i], strlen(argv[i]));
        memcpy(buf + size + strlen(argv[i]), sep, 1);
        size += strlen(argv[i]);
        size ++;
    }
    write(sockfd, buf, strlen(buf));
    bzero(buf, sizeof(buf));
    i = read(sockfd, buf, sizeof(buf));
    write(1, buf, i);
    close(sockfd);
    return 0;
}
