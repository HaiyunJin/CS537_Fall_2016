
#include <stdio.h>
#include <unistd.h>

int main () {
    char *s[] = {"/bin/ls",NULL};
//    char* arg[] = {"ls",NULL};
    printf("Enter sth.: ");
    printf("%s\n",s[0]);
    if (fork() == 0) {
        if (execvp(s[0],s) < 0 ) {
 //   if (execvp(arg[0],arg) < 0 ) {
            printf("invalid\n");
        }
    }
    printf("\n\n\n\nend\n");
    return 1;
}

