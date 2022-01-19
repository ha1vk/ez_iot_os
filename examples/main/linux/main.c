#include <stdio.h>
#include <unistd.h>

int main()
{
    printf("Hello world!");
    while (1)
    {
        sleep(1000);
    }
    
    return 0;
}