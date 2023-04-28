#include "stak.h"

#include <time.h>

bool reload = 0;

int
main(int argc, char *argv[])
{
    srand(time(NULL));

    variable_ll_init(&variables[0]);
    audio_init();

    parser_file("example.st");

    {
        char ch;
        while((ch = getchar()) != 'q')
            {
                if(ch == 'r')
                    parser_file("example.st");
                variable_ll_print(&variables[0]);
            }
    }

    audio_deinit();
    variable_ll_deinit(&variables[0]);
    return 0;
}
