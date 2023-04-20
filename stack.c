#include "stak.h"

enum status
duplicate(struct stack *s)
{
    if(s->pos == 0)
        return BAD;

    s->pos++;
    s->data[s->pos - 1] = s->data[s->pos - 2];

    return OK;
}

struct value
pop(struct stack *s)
{
    if(s->pos == 0)
        {
            struct value err = { VALUE_ERROR };
            return err;
        }

    return s->data[--s->pos];
}

float
popf(struct stack *s)
{
    if(check(s, VALUE_FLOAT, 0))
        return pop(s).x.f;
    else
        return 0;
}

enum status
push(struct stack *s, struct value v)
{
    if(s->pos == MAX_STACK_SIZE - 1)
        return BAD;

    s->data[s->pos++] = v;

    return OK;
}

enum status
take(struct stack *src, struct stack *dest, int num)
{
    int i;

    if(src->pos < MAX_STACK_SIZE - 1 - num)
        return BAD;
    if(dest->pos >= MAX_STACK_SIZE - 1 - num)
        return BAD;

    for(i = 0; i < num; ++i)
        dest->data[dest->pos++] = src->data[src->pos - num + i + 1];

    return OK;
}

enum status
concat(struct stack *src, struct stack *dest)
{
    int i;

    if(dest->pos + src->pos >= MAX_STACK_SIZE)
        return BAD;

    for(i = 0; i < src->pos; ++i)
        dest->data[dest->pos++] = src->data[i];

    return OK;
}

bool
check(struct stack *s, enum value_type type, int offset)
{
    if(s->pos <= offset)
        return false;

    return s->data[s->pos - 1 - offset].type == type;
}

void
print_stack(struct stack *s)
{
    int i;
    for(i = 0; i < s->pos; ++i)
        {
            struct value *v = &s->data[i];

            printf("%s ", value_type_name[v->type]);

            switch(v->type)
                {
                    case VALUE_FLOAT:
                        printf("%f", v->x.f);
                        break;

                    case VALUE_STRING:
                        printf("%s", (char*)v->x.p);
                        break;

                    default:
                        break;
                }

            printf(" ");
        }
    printf("\n");
}
