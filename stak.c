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
#define MAX_INPUT_LENGTH 100
        char buffer[MAX_INPUT_LENGTH];

        printf("$ ");
        while(fgets(buffer, MAX_INPUT_LENGTH, stdin))
            {
                if(strcmp(buffer, "r\n") == 0)
                    parser_file("example.st");
                else if(strcmp(buffer, "q\n") == 0)
                    break;
                else if(strcmp(buffer, "\n") == 0)
                    variable_ll_print(&variables[0]);
                else
                    parser_string(buffer);

                printf("$ ");
            }
#undef MAX_INPUT_LENGTH
    }

    audio_deinit();
    variable_ll_deinit(&variables[0]);
    return 0;
}
