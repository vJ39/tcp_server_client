#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/capability.h>

int main(void){
    int fd;
    cap_rights_t rights;

    if((fd = open("test", O_CREAT|O_TRUNC|O_WRONLY)) == -1 )
        perror("open");

    if( cap_enter() == -1)
        perror("cap_error");

    cap_rights_init(&rights, CAP_READ);

    if( cap_rights_limit(fd, &rights) == -1 )
        perror("cap_rights_limit");

    if( write(fd, "hoge\n", 5) == -1 )
        perror("write");

    if( close(fd) == -1 )
        perror("close");

    return 0;
}
