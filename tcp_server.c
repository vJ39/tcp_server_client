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
#include <sys/resource.h>
#include <signal.h>
#define ROOTDIR "/jail/echo"
#define JAIL 1
#define ADDR "192.168.1.106"
#define MAXWORKERS 50
void setup_signal_handler(void);
void catch_signal(int);
int main() {
    int server_sockfd, client_sockfd, server_len, client_len, i;
    size_t j;
    fd_set ready;
    struct sockaddr_in server_address, client_address;
    struct timeval to;
    char *mkargv[10];
    int pid;
    char buf[1024*1024];
    struct in_addr addr;
    inet_aton(ADDR, &addr);
    if( chdir(ROOTDIR) == -1 ) perror("chdir");
#ifdef JAIL
    /* RLIMIT */
    struct rlimit rl;
    int rc, jid;
    rl.rlim_cur = 30;
    rl.rlim_max = 60;
    rc = setrlimit(RLIMIT_NPROC, &rl);

    rl.rlim_cur = 2;
    rl.rlim_max = 4;
    rc = setrlimit(RLIMIT_CPU, &rl);

    rl.rlim_cur = 50*1024*1024;
    rl.rlim_max = 100*2048*1024;
    rc = setrlimit(RLIMIT_VMEM, &rl);

    /* JAIL */
    int securelevel = 2;
    int childrenmax = 10;

    struct iovec iov[12];
    iov[0].iov_base = "name";
    iov[0].iov_len  = sizeof("name");
    iov[1].iov_base = "echo_service";
    iov[1].iov_len  = sizeof("echo_service");
    iov[2].iov_base = "host.hostname";
    iov[2].iov_len  = sizeof("host.hostname");
    iov[3].iov_base = "echo.example.jp";
    iov[3].iov_len  = sizeof("echo.example.jp");
    iov[4].iov_base = "path";
    iov[4].iov_len  = sizeof("path");
    iov[5].iov_base = ROOTDIR;
    iov[5].iov_len  = sizeof(ROOTDIR);
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
    if( (jid = jail_set(iov, 12, JAIL_CREATE | JAIL_ATTACH | JAIL_DYING)) == -1) {
        if((jid = jail_set(iov, 2, JAIL_UPDATE | JAIL_ATTACH | JAIL_DYING)) == -1)
            perror("jail_set(update)");
        else
            perror("jail_set(create)");
    }
    printf("Master Jail ID = %d\n", jid);
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

    int masterpid;
    for(i = 0; i < 10; i++) {
        if((masterpid = fork()) == 0) {
            while(1) {
                FD_ZERO(&ready);
                FD_SET(server_sockfd, &ready);
                to.tv_sec = 60;
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
                        if(ptr + i - buf > j) {mkargv[i] = NULL; continue;}
                        mkargv[i] = (char *)malloc(strlen(ptr)+1);
                        memcpy(mkargv[i], ptr, strlen(ptr));
                        ptr += strlen(ptr);
                        ptr ++;
                    }

                    // redirect
                    if(close(2) == -1) perror("close(2)");
                    if(close(1) == -1) perror("close(1)");
                    if(close(0) == -1) perror("close(0)");
                    if(dup2(client_sockfd, 2) == -1) perror("dup2(2)");
                    if(dup2(client_sockfd, 1) == -1) perror("dup2(1)");
                    if(dup2(client_sockfd, 0) == -1) perror("dup2(0)");

                    bzero(buf, sizeof(buf));

                    if(!strncmp(mkargv[0], "jailremove", strlen("jailremove"))) {
                        jid = atoi(mkargv[1]);
                        printf("jailremove %d\n", jid);
                        if(jail_remove(jid) == -1) perror("jail_remove");
                        shutdown(client_sockfd, SHUT_WR);
                        close(client_sockfd);
                        continue;
                    }
                    if((pid = fork()) == 0) {
                        struct iovec childiov[4];
                        childiov[0].iov_base = "name";
                        childiov[0].iov_len  = sizeof("name");
                        childiov[1].iov_base = "echo_child_service";
                        childiov[1].iov_len  = sizeof("echo_child_service");
                        childiov[2].iov_base = "path";
                        childiov[2].iov_len  = sizeof("path");
                        childiov[3].iov_base = "/";
                        childiov[3].iov_len  = sizeof("/");
                        if( (jid = jail_set(childiov, 4, JAIL_CREATE | JAIL_ATTACH | JAIL_DYING )) == -1) {
                            if((jid = jail_set(childiov, 2, JAIL_UPDATE | JAIL_ATTACH | JAIL_DYING)) == -1) {
                                perror("jail(child)(update)");
                                exit(0);
                            }
                            else perror("jail(child)(create)");
                        }
                        printf("JAIL ID = %d\n", jid);
                        if( setgid((gid_t)2) == -1 ) perror("setgid");
                        if( setuid((uid_t)2) == -1 ) perror("setuid");
                        if(execve(mkargv[0], mkargv, NULL) == -1) perror("execve");
                        for(i = 0;i < 10; i++) free(mkargv[i]);
                    }
                    else if(pid < 0) perror("fork");
                    else {
                        int status;
                        waitpid(-1, &status, 0);
                    }
                    shutdown(client_sockfd, SHUT_WR);
                    close(client_sockfd);
                }
            }
        }
        else if(masterpid < 0) {
            perror("fork");
            exit(0);
        }
    }
    return 0;
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
    act.sa_handler = catch_signal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NOCLDSTOP | SA_RESTART;
    if(sigaction(SIGCHLD, &act, 0) == -1) perror("sigaction CHLD");
}

void catch_signal(int signum){
    int status;
    if(waitpid(-1, &status, WNOHANG) < 0) perror("waitpid(catch_signal)");
}
