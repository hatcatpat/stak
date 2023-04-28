#include "stak.h"

void
value_print(value_t *value)
{
    switch(value->type)
        {
            case VALUE_ERROR:
                printf("(ERROR) ");
                break;

            case VALUE_NUMBER:
                printf("(NUMBER %f) ", value->x.number);
                break;

            case VALUE_STRING:
                if(value->x.pointer == NULL)
                    printf("(STRING NULL) ");
                else
                    printf("(STRING %s) ", (char *)value->x.pointer);
                break;

            case VALUE_VARIABLE:
                if(value->x.pointer == NULL)
                    printf("(VARIABLE NULL) ");
                else
                    printf("(VARIABLE %s) ", ((variable_t *)value->x.pointer)->key);
                break;

            case VALUE_BUFFER:
                if(value->x.pointer == NULL)
                    printf("(BUFFER NULL) ");
                else
                    {
                        buffer_t *buffer = value->x.pointer;
                        printf("(BUFFER len: %lu, chans: %i, data: %c) ", buffer->len, buffer->chans, buffer->data == NULL ? 'n' : 'y');
                    }
                break;
        }
}

void
stack_print(stack_t *stack)
{
    printf("\t[stack] pos: %i\n", stack->pos);

    if(stack->pos > 0)
        {
            int i;

            printf("\t\t");

            for(i = 0; i < stack->pos; ++i)
                value_print(&stack->values[i]);

            printf("\n");
        }
}

void
stack_push(stack_t *stack, value_t value)
{
    if(stack->pos >= STACK_SIZE)
        return;

    stack->values[stack->pos++] = value;
}

value_t
stack_pop(stack_t *stack)
{
    if(stack->pos == 0)
        {
            value_t v = { VALUE_ERROR };
            return v;
        }

    return stack->values[--stack->pos];
}

float
stack_pop_number(stack_t *stack)
{
    value_t value = stack_pop(stack);
    return (value.type == VALUE_NUMBER) ? value.x.number : 0;
}

void *
stack_pop_pointer(stack_t *stack)
{
    value_t value = stack_pop(stack);
    return (value.type != VALUE_NUMBER && value.type != VALUE_ERROR) ? value.x.pointer : NULL;
}

value_t *
stack_peek(stack_t *stack)
{
    if(stack->pos == 0)
        return NULL;

    return &stack->values[stack->pos - 1];
}

float
stack_peek_number(stack_t *stack)
{
    value_t *value = stack_peek(stack);
    return (value->type == VALUE_NUMBER) ? value->x.number : 0;
}

void *
stack_peek_pointer(stack_t *stack)
{
    value_t *value = stack_peek(stack);
    return (value->type != VALUE_NUMBER && value->type != VALUE_ERROR) ? value->x.pointer : NULL;
}

void
stack_concat(stack_t *dest, stack_t *source)
{
    int i;

    if(dest->pos + source->pos >= STACK_SIZE)
        return;

    for(i = 0; i < source->pos; ++i)
        dest->values[dest->pos++] = source->values[i];
}

void
stack_reset(stack_t *stack)
{
    stack->pos = 0;
}

void
stack_dup(stack_t *stack)
{
    stack_push(stack, *stack_peek(stack));
}

void
stack_swap(stack_t *stack)
{
    value_t value;

    if(stack->pos < 2)
        return;

    value = stack->values[stack->pos - 1];
    stack->values[stack->pos - 1] = stack->values[stack->pos - 2];
    stack->values[stack->pos - 2] = value;
}

void
stack_rot(stack_t *stack)
{
    value_t value;

    if(stack->pos < 3)
        return;

    value = stack->values[stack->pos - 1];
    stack->values[stack->pos - 1] = stack->values[stack->pos - 2];
    stack->values[stack->pos - 2] = stack->values[stack->pos - 3];
    stack->values[stack->pos - 3] = value;

}

void
stack_drop(stack_t *stack)
{
    (void)stack_pop(stack);
}
