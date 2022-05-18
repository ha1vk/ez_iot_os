#include "ezos.h"
#include <signal.h>

int main(int argc, char **argv)
{
    ezos_printf("ezapp, easy your life!\r\n");

    while (1)
    {
        ezos_delay_ms(1000);
    }
}