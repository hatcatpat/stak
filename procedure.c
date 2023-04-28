#include "stak.h"

void
procedure_print(procedure_t *procedure)
{
    printf("\t[procedure] len: %lu, process_type: %i\n", procedure->len, procedure->process_type);

    if(procedure->atoms != NULL)
        {
            int i;

            printf("\t\t");

            for(i = 0; i < procedure->len; ++i)
                atom_print(&procedure->atoms[i]);

            printf("\n");
        }
}

void
procedure_deinit(procedure_t *procedure)
{
    if(procedure->atoms != NULL)
        {
            int i;

            for(i = 0; i < procedure->len; ++i)
                atom_deinit(&procedure->atoms[i]);

            free(procedure->atoms);
        }

    memset(procedure, 0, sizeof(procedure_t));
}

void
procedure_push(procedure_t *procedure, atom_t atom)
{
    procedure->len++;
    procedure->atoms = realloc(procedure->atoms, procedure->len * sizeof(atom_t));
    procedure->atoms[procedure->len - 1] = atom;

    procedure_reset_process_type(procedure);
}

void
procedure_process(procedure_t *procedure, stack_t *stack)
{
    int i;

    if(procedure->atoms == NULL || procedure->process_type == PROCESS_DONE)
        return;

    if(procedure->process_type == PROCESS_ONCE)
        procedure->process_type = PROCESS_DONE;

    stack_reset(stack);

    for(i = 0; i < procedure->len; ++i)
        atom_process(&procedure->atoms[i], stack);
}

void
procedure_reset_process_type(procedure_t *procedure)
{
    int i;

    if(procedure->atoms == NULL)
        {
            procedure->process_type = PROCESS_DONE;
            return;
        }

    procedure->process_type = PROCESS_ONCE;

    for(i = 0; i < procedure->len; ++i)
        {
            atom_t *atom = &procedure->atoms[i];

            switch(atom->type)
                {
                    case ATOM_FUNCTION:
                        if(atom->x.function.process_type == PROCESS_ONCE)
                            {
                                procedure->process_type = PROCESS_ONCE;
                                return;
                            }
                        else if(atom->x.function.process_type == PROCESS_ALWAYS)
                            procedure->process_type = PROCESS_ALWAYS;
                        break;

                    case ATOM_VARIABLE:
                        if(atom->x.variable != NULL)
                            if(atom->x.variable->procedure.process_type == PROCESS_ALWAYS)
                                procedure->process_type = PROCESS_ALWAYS;
                        break;

                    default:
                        break;
                }
        }
}
