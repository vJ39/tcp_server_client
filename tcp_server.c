#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/jail.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#define ROOTDIR "/jail/echo"
// #define JAIL 1
void setup_signal_handler(void);
void cache_signal(int);
int main() {
    int server_sockfd, client_sockfd, server_len, client_len, i;
    struct sockaddr_in server_address, client_address;
    char *mkargv[10];
    int pid, status;
    char buf[1024];
    struct in_addr addr;
#ifdef JAIL
    int jid;
    struct iovec iov[8];
    iov[0].iov_base = "path";
    iov[0].iov_len  = sizeof("path");
    iov[1].iov_base = ROOTDIR;
    iov[1].iov_len  = sizeof(ROOTDIR);
    iov[2].iov_base = "host.hostname";
    iov[2].iov_len  = sizeof("host.hostname");
    iov[3].iov_base = "echo.example.jp";
    iov[3].iov_len  = sizeof("echo.example.jp");
    iov[4].iov_base = "name";
    iov[4].iov_len  = sizeof("name");
    iov[5].iov_base = "echo_service";
    iov[5].iov_len  = sizeof("echo_service");
    iov[6].iov_base = "ip4.addr";
    iov[6].iov_len  = sizeof("ip4.addr");
    iov[7].iov_base = &addr;
    iov[7].iov_len  = sizeof addr;
    inet_aton("192.168.1.106", &addr);
    jid = jail_set(iov, 8, JAIL_CREATE | JAIL_ATTACH);

    printf("Jail ID = %d\n", jid);
#else
    if( chdir(ROOTDIR) == -1 ) perror("chdir");
    if( chroot(ROOTDIR) == -1 ) perror("chroot");
#endif
    server_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    server_address.sin_family = AF_INET;
    server_address.sin_addr = addr;
    server_address.sin_port = htons(9000);
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *) &server_address, server_len);
    listen(server_sockfd, 5);

    setup_signal_handler();

    while(1) {
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, (socklen_t *)&client_len);
        bzero(buf, sizeof(buf));
        if(read(client_sockfd, buf, sizeof(buf)) == -1) perror("read");

        const char sep[] = {0x0a};
        char *ptr = buf;
        for(i = 0; i < 10; i++){
            if( (ptr = mkargv[i] = strtok(ptr, sep)) == NULL) break;
            ptr += strlen(ptr) + 1;
        }
        for(; i < 10; i++) mkargv[i] = NULL;

        // redirect
        /*
        if(close(0) == -1) perror("close(0)");
        if(close(1) == -1) perror("close(1)");
        if(close(2) == -1) perror("close(2)");
        */
        if(dup2(client_sockfd, 0) == -1) perror("dup2(0)");
        if(dup2(client_sockfd, 1) == -1) perror("dup2(1)");
        if(dup2(client_sockfd, 2) == -1) perror("dup2(2)");

        if((pid = fork()) < 0) perror("fork");
        else if(!pid) {
            if(execve(mkargv[0], mkargv, NULL) == -1) perror("execve");
        }
        else {
            waitpid(pid, &status, WUNTRACED);
        }
        shutdown(client_sockfd, SHUT_WR);
        close(client_sockfd);
    }
}

void setup_signal_handler(){
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    act.sa_handler = cache_signal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NOCLDSTOP | SA_RESTART;
    if(sigaction(SIGPIPE, &act, 0) == -1) perror("sigaction");
    if(sigaction(SIGCHLD, &act, 0) == -1) perror("sigaction");
}

void cache_signal(int signum){
    int status;
    waitpid(-1, &status, WNOHANG);
}
