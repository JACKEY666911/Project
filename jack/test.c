#include<stdio.h>
#include<stdlib.h>
int main()
{
    char *path = getenv("PWD");
    printf("%s\n",path);
}
