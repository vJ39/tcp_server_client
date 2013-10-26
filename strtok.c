#include <string.h>
#include <stdio.h>
int main(){
    char *str[10];
    const char sep[] = "o";
    char buf[] = "hogehoge";
    str[0] = strtok(buf, sep);
    str[1] = strtok((char *)(buf + *str[0]), sep);
    str[2] = strtok((char *)(buf + *str[1]), sep);
    printf("%d\n", (int)*str[0]);
    printf("%d\n", (int)*str[1]);
    if(str[2] != NULL)
    printf("%d\n", (int)*str[2]);
    return 0;
}
