#include "stak.h"

#define STAK_FUNC(name) static enum status name(struct stack *s)

STAK_FUNC(add) /* a b -- (a + b) */
{
    struct value v = { VALUE_FLOAT };
    v.x.f = popf(s) + popf(s);
    push(s, v);
    return OK;
}

STAK_FUNC(sub) /* a b -- (a - b) */
{
    struct value v = { VALUE_FLOAT };
    float b = popf(s);
    float a = popf(s);
    v.x.f = a - b;
    push(s, v);
    return OK;
}

STAK_FUNC(mul) /* a b -- (a * b) */
{
    struct value v = { VALUE_FLOAT };
    v.x.f = popf(s) * popf(s);
    push(s, v);
    return OK;
}

STAK_FUNC(divi) /* a b -- (a / b) */
{
    struct value v = { VALUE_FLOAT };
    float b = popf(s);
    float a = popf(s);
    v.x.f = b ? a / b : 0;
    push(s, v);
    return OK;
}

STAK_FUNC(map) /* x lo hi -- y */
{
    struct value v = { VALUE_FLOAT };
    float hi = popf(s);
    float lo = popf(s);
    float x = popf(s);
    v.x.f = x * (hi - lo) + lo;
    push(s, v);
    return OK;
}

STAK_FUNC(bi2norm) /* x -- y */
{
    struct value v = { VALUE_FLOAT };
    float bi = popf(s);
    v.x.f = (bi + 1) * 0.5;
    push(s, v);
    return OK;
}

STAK_FUNC(norm2bi) /* x -- y */
{
    struct value v = { VALUE_FLOAT };
    float norm = popf(s);
    v.x.f = norm * 2 - 1;
    push(s, v);
    return OK;
}

STAK_FUNC(pi) /* -- pi */
{
    static struct value v = { VALUE_FLOAT, { PI } };
    push(s, v);
    return OK;
}

STAK_FUNC(tau) /* -- tau */
{
    static struct value v = { VALUE_FLOAT, { TAU } };
    push(s, v);
    return OK;
}

STAK_FUNC(pan) /* x pan -- left right */
{
    struct value v = { VALUE_FLOAT };
    float p = (popf(s) + 1) * 0.5;
    float x = popf(s);

    v.x.f = x * p;
    push(s, v);

    v.x.f = x * (1 - p);
    push(s, v);

    return OK;
}

STAK_FUNC(mix) /* l1 r1 l2 r2 -- (l1 + l2) (r1 + r2) */
{
    struct value v = { VALUE_FLOAT };
    float l1, l2, r1, r2;
    r2 = popf(s);
    l2 = popf(s);
    r1 = popf(s);
    l1 = popf(s);

    v.x.f = l1 + l2;
    push(s, v);

    v.x.f = r1 + r2;
    push(s, v);

    return OK;
}

STAK_FUNC(m2f) /* midi -- freq */
{
    struct value v = { VALUE_FLOAT };
    v.x.f = midi2freq(popf(s));
    push(s, v);
    return OK;
}

STAK_FUNC(f2m) /* freq -- midi */
{
    struct value v = { VALUE_FLOAT };
    v.x.f = freq2midi(popf(s));
    push(s, v);
    return OK;
}

struct funcs_
{
    char *key;
    function fn;
} funcs[] =
{
    {"add", add},
    {"+", add},
    {"sub", sub},
    {"-", sub},
    {"mul", mul},
    {"*", mul},
    {"div", divi},
    {"/", divi},
    {"map", map},
    {"bi2norm", bi2norm},
    {"norm2bi", norm2bi},
    {"pi", pi},
    {"tau", tau},
    {"dup", duplicate},
    {"pan", pan},
    {"mix", mix},
    {"m2f", m2f},
    {"f2m", f2m},
};

size_t num_funcs = sizeof(funcs) / sizeof(struct funcs_);

void
print_funcs()
{
    int i;
    for(i = 0; i < num_funcs; ++i)
        printf("[func %i] %s\n", i, funcs[i].key);
}

char *
get_func_name(function fn)
{
    int i;
    for(i = 0; i < num_funcs; ++i)
        if(fn == funcs[i].fn)
            return funcs[i].key;

    return "ERROR";
}

function
find_func(char *key)
{
    int i;

    for(i = 0; i < num_funcs; ++i)
        if(strcmp(key, funcs[i].key) == 0)
            return funcs[i].fn;

    return NULL;
}
