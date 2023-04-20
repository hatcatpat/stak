#include "stak.h"

#include <signal.h>

bool reload = false;
struct variable_ll new_root;

const char *atom_type_name[] =
{
    "ERROR",
    "FLOAT",
    "STRING",
    "FUNCTION",
    "UGEN",
    "VARIABLE"
};

const char *value_type_name[] =
{
    "ERROR",
    "FLOAT",
    "STRING",
    "BUFFER"
};

void
print_atom(struct atom *a)
{
    printf("%s ", atom_type_name[a->type]);

    switch(a->type)
        {
            case ATOM_FLOAT:
                printf("%f", a->x.f);
                break;

            case ATOM_STRING:
                printf("%s", a->x.s);
                break;

            case ATOM_UGEN:
                printf("%s", get_ugen_name(&a->x.u));
                break;

            case ATOM_VARIABLE:
                printf("%s", a->x.v.key);
                break;

            case ATOM_FUNCTION:
                printf("%s", get_func_name(a->x.fn));
                break;

            default:
                break;
        }

    printf(" ");
}

void
sigint(int x)
{
    server_running = false;
}

int
main(void)
{
    print_ugens();
    print_funcs();

    init_notes();
    init_audio();

    parse_file("test.st");

    signal(SIGINT, &sigint);

    run_server();

    deinit_audio();
    deinit_vars(&stak_root);
    return 0;
}

