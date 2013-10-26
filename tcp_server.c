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
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#define ROOTDIR "/jail/echo"
#define JAIL 1
void setup_signal_handler(void);
void cache_signal(int);
void termination(int);
int server_sockfd;

int main() {
    int client_sockfd, server_len, client_len, i, j;
    fd_set ready;
    struct sockaddr_in server_address, client_address;
    struct timeval to;
    char *mkargv[10];
    int pid, status;
    char buf[1024*1024];
    struct in_addr addr;
    inet_aton("192.168.1.106", &addr);
    if( chdir(ROOTDIR) == -1 ) perror("chdir");
#ifdef JAIL
    int jid;

    int securelevel = 2;
    int childrenmax = 1;

    struct iovec iov[12];
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
    iov[7].iov_len  = sizeof(addr);
    iov[8].iov_base = "children.max";
    iov[8].iov_len  = sizeof("children.max");
    iov[9].iov_base = &childrenmax;
    iov[9].iov_len  = sizeof(childrenmax);
    iov[10].iov_base = "securelevel";
    iov[10].iov_len  = sizeof("securelevel");
    iov[11].iov_base = &securelevel;
    iov[11].iov_len  = sizeof(securelevel);
    if( (jid = jail_set(iov, 12, JAIL_CREATE | JAIL_ATTACH)) == -1) perror("jail");

    printf("Jail ID = %d\n", jid);
#else
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
        FD_ZERO(&ready);
        FD_SET(server_sockfd, &ready);
        to.tv_sec = 1;
        to.tv_usec = 0;
        if(select(server_sockfd + 1, &ready, (fd_set *)0, (fd_set *)0, &to) == -1) {
            perror("select");
            continue;
        }
        if(FD_ISSET(server_sockfd, &ready)) {
            client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, (socklen_t *)&client_len);
            bzero(buf, sizeof(buf));
            if((j = read(client_sockfd, buf, sizeof(buf))) == -1) perror("read");

            char *ptr = buf;
            for(i = 0; i < 10; i++){
                if(ptr + i + 1 - buf > j) break;
                memcpy(mkargv[i], ptr, strlen(ptr));
                ptr += strlen(ptr);
                ptr ++;
            }
            for(; i < 10; i++) mkargv[i] = NULL;

            // redirect
            if(close(2) == -1) perror("close(2)");
            if(close(1) == -1) perror("close(1)");
            if(close(0) == -1) perror("close(0)");
            if(dup2(client_sockfd, 2) == -1) perror("dup2(2)");
            if(dup2(client_sockfd, 1) == -1) perror("dup2(1)");
            if(dup2(client_sockfd, 0) == -1) perror("dup2(0)");

            if((pid = fork()) < 0) perror("fork");
            else if(!pid) {
                iov[0].iov_base = "path";
                iov[0].iov_len  = sizeof("path");
                iov[1].iov_base = "/";
                iov[1].iov_len  = sizeof("/");
                iov[2].iov_base = "name";
                iov[2].iov_len  = sizeof("name");
                iov[3].iov_base = "echo_child_service";
                iov[3].iov_len  = sizeof("echo_child_service");
                if( (jid = jail_set(iov, 4, JAIL_CREATE | JAIL_ATTACH)) == -1) perror("jail");
                if( setgid((gid_t)2) == -1 ) perror("setgid");
                if( setuid((uid_t)2) == -1 ) perror("setuid");
                if(execve(mkargv[0], mkargv, NULL) == -1) perror("execve");
            }
            else {
                waitpid(pid, &status, WUNTRACED);
            }
            shutdown(client_sockfd, SHUT_WR);
            close(client_sockfd);
        }
    }
}

void setup_signal_handler(){
    struct sigaction act;
    // SIGPIPE
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    sigemptyset(&act.sa_mask);
    if(sigaction(SIGPIPE, &act, 0) == -1) perror("sigaction SIGPIPE");

    // SIGCHLD
    memset(&act, 0, sizeof(act));
    act.sa_handler = cache_signal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NOCLDSTOP | SA_RESTART;
    if(sigaction(SIGCHLD, &act, 0) == -1) perror("sigaction CHLD");

    // SIGQUIT
    memset(&act, 0, sizeof(act));
    act.sa_handler = termination;
    sigemptyset(&act.sa_mask);
    if(sigaction(SIGQUIT, &act, 0) == -1) perror("sigaction SIGQUIT");
}

void cache_signal(int signum){
    int status;
    waitpid(-1, &status, WNOHANG);
}

void termination(int signum){
    shutdown(server_sockfd, SHUT_RD);
    close(server_sockfd);
}
