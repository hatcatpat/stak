#include "stak.h"

enum status ugen_sine_init(struct ugen *u);
enum status ugen_saw_init(struct ugen *u);
enum status ugen_pul_init(struct ugen *u);
enum status ugen_tri_init(struct ugen *u);

void ugen_osc_deinit(struct ugen *u);
enum status ugen_osc_update(struct stack *s, struct ugen *u);

struct ugens_
{
    char *key;
    struct ugen u;
} ugens[] =
{
    { "sine", { ugen_sine_init, ugen_osc_deinit, ugen_osc_update } },
    { "saw", { ugen_saw_init, ugen_osc_deinit, ugen_osc_update } },
    { "pul", { ugen_pul_init, ugen_osc_deinit, ugen_osc_update } },
    { "tri", { ugen_tri_init, ugen_osc_deinit, ugen_osc_update } },
};

size_t num_ugens = sizeof(ugens) / sizeof(struct ugens_);

void
print_ugens()
{
    int i;
    for(i = 0; i < num_ugens; ++i)
        printf("[ugen %i] %s\n", i, ugens[i].key);
}

char *
get_ugen_name(struct ugen *u)
{
    int i;
    for(i = 0; i < num_ugens; ++i)
        if(u->init == ugens[i].u.init
                && u->deinit == ugens[i].u.deinit
                && u->update == ugens[i].u.update)
            return ugens[i].key;

    return "ERROR";
}

struct ugen*
find_ugen(char *key)
{
    int i;

    for(i = 0; i < num_ugens; ++i)
        if(strcmp(key, ugens[i].key) == 0)
            return &ugens[i].u;

    return NULL;
}

/* osc */
typedef float (*wave)(float x);
struct ugen_osc
{
    wave wave;
    float phase;
};

float
sine(float x)
{
    return sin(x);
    /* return 1 - (20 * x * x) / (4 * x * x + TAU * TAU); */
}

float
saw(float x)
{
    return x / TAU;
}

float
pul(float x)
{
    return x < PI ? -1 : 1;
}

float
tri(float x)
{
    if(x < PI)
        return (x / PI) * 2 - 1;
    else
        return -(((x - PI) / PI) * 2 - 1);
}

enum status
ugen_osc_init(struct ugen *u, float (*wave)(float x))
{
    u->data = calloc(1, sizeof(struct ugen_osc));
    ((struct ugen_osc *)u->data)->wave = wave;

    if(u->data)
        return OK;
    else
        return BAD;
}

enum status
ugen_sine_init(struct ugen *u)
{
    return ugen_osc_init(u, sine);
}
enum status
ugen_saw_init(struct ugen *u)
{
    return ugen_osc_init(u, saw);
}
enum status
ugen_pul_init(struct ugen *u)
{
    return ugen_osc_init(u, pul);
}
enum status
ugen_tri_init(struct ugen *u)
{
    return ugen_osc_init(u, tri);
}

void
ugen_osc_deinit(struct ugen *u)
{
    if(u->data)
        {
            free(u->data);
            u->data = NULL;
        }
}

enum status
ugen_osc_update(struct stack *s, struct ugen *u)
{
    struct ugen_osc *osc = u->data;
    struct value v = { VALUE_FLOAT };
    float freq = popf(s);

    v.x.f = osc->wave(osc->phase * TAU);
    push(s, v);

    osc->phase += freq / audio.rate;
    osc->phase = fmod(osc->phase, 1.0);

    return OK;
}
