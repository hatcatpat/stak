#include "stak.h"

#include <time.h>

bool reload = 0;
bool running = 0;

#define MAX_INPUT_LENGTH 100
void
user_input()
{
    static char buffer[MAX_INPUT_LENGTH];

    printf("$ ");
    while(running)
        {
            fgets(buffer, MAX_INPUT_LENGTH, stdin);
            parser_command(buffer);
            printf("$ ");
        }
}
#undef MAX_INPUT_LENGTH

int
main(int argc, char *argv[])
{
    srand(time(NULL));

    variable_ll_init(&variables[0]);
    audio_init();

    running = 1;

    parser_file("example.st");

#ifdef UDP
    udp_run();
#else
    user_input();
#endif

    audio_deinit();
    variable_ll_deinit(&variables[0]);

    return 0;
}
