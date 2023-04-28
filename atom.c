#include "stak.h"

void
atom_print(atom_t *atom)
{
    switch(atom->type)
        {
            case ATOM_ERROR:
                printf("(ERROR) ");
                break;

            case ATOM_FUNCTION:
                printf("(FUNCTION %s) ", function_find_key(&atom->x.function));
                break;

            case ATOM_NUMBER:
                printf("(NUMBER %f) ", atom->x.number);
                break;

            case ATOM_STRING:
                printf("(STRING %s) ", atom->x.string);
                break;

            case ATOM_VARIABLE:
                printf("(VARIABLE %s) ", atom->x.variable->key);
                break;
        }
}

void
atom_init(atom_t *atom)
{
    if(atom->type == ATOM_FUNCTION)
        {
            atom->x.function.data = NULL;

            if(atom->x.function.init != NULL)
                atom->x.function.init(&atom->x.function.data);
        }
}

void
atom_deinit(atom_t *atom)
{
    if(atom->type == ATOM_FUNCTION)
        {
            if(atom->x.function.deinit != NULL)
                atom->x.function.deinit(&atom->x.function.data);

            if(atom->x.function.data != NULL)
                {
                    free(atom->x.function.data);
                    atom->x.function.data = NULL;
                }
        }
    else if(atom->type == ATOM_STRING)
        if(atom->x.string != NULL)
            {
                free(atom->x.string);
                atom->x.string = NULL;
            }
}

void
atom_process(atom_t *atom, stack_t *stack)
{
    value_t value;

    switch(atom->type)
        {
            case ATOM_FUNCTION:
                if(atom->x.function.process != NULL)
                    atom->x.function.process(&atom->x.function.data, stack);
                break;

            case ATOM_VARIABLE:
                if(atom->x.variable == NULL)
                    {
                        value.type = VALUE_ERROR;
                        stack_push(stack, value);
                    }
                else
                    stack_concat(stack, &atom->x.variable->stack);
                break;

            case ATOM_NUMBER:
                value.type = VALUE_NUMBER;
                value.x.number = atom->x.number;
                stack_push(stack, value);
                break;

            case ATOM_STRING:
                value.type = VALUE_STRING;
                value.x.pointer = atom->x.string;
                stack_push(stack, value);
                break;

            case ATOM_ERROR:
                value.type = VALUE_ERROR;
                stack_push(stack, value);
                break;
        }
}
