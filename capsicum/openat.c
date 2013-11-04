#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/capability.h>

int main(int argc, char **argv){
    int fd, pfd;
    cap_rights_t rights;

    if( (fd = open("/bin", O_RDONLY)) == -1 )
        perror("open");

    if( (pfd = openat(fd, "ls", O_CLOEXEC)) == -1 )
        perror("openat");

    cap_rights_init(&rights, CAP_ALL0);
    if( cap_rights_limit(pfd, &rights) == -1 )
        perror("cap_rights_limit");

    cap_enter();

    if( close(fd) == -1 )
        perror("close");

    if( fexecve(pfd, argv, NULL) == -1 )
        perror("fexecve");

    return 0;
}
