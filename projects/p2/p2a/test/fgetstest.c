
#include <stdio.h>

int main () {
    char s[100];
    printf("Enter sth.: ");
    fgets(s,100,stdin);
    printf("%s",s);
}

