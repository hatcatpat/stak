#include "stak.h"

#include <math.h>

/* ugens */
typedef struct _oscil
{
    int wave;
    float phase;
} oscil_t;

typedef float (*wave_t)(float phase);

float
wave_sine(float phase)
{
    return sin(phase);
}

float
wave_saw(float phase)
{
    return (phase / TAU) * 2 - 1;
}

float
wave_tri(float phase)
{
    if(phase < PI)
        return (phase / PI) * 2 - 1;
    else
        return -(((phase - PI) / PI) * 2 - 1);
}

float
wave_pulse(float phase)
{
    return phase < PI ? -1 : 1;
}

#define NUM_WAVES 4
wave_t waves[NUM_WAVES] =
{
    wave_sine,
    wave_saw,
    wave_tri,
    wave_pulse
};

void
ugen_oscil_init(void **data)
{
    *data = calloc(1, sizeof(oscil_t));
}

void
ugen_oscil_process_general(void **data, stack_t *stack, uint8_t wave)
{
    value_t value = { VALUE_NUMBER };
    oscil_t *oscil = *data;
    float freq = stack_pop_number(stack);

    value.x.number = waves[wave % NUM_WAVES](oscil->phase);
    stack_push(stack, value);

    oscil->phase += TAU * ABS(freq) / audio.rate;
    oscil->phase = fmod(oscil->phase, TAU);
}

void
ugen_oscil_process(void **data, stack_t *stack) /* freq wave -- output */
{
    ugen_oscil_process_general(data, stack, stack_pop_number(stack));
}

void
ugen_sin_process(void **data, stack_t *stack) /* freq -- output */
{
    ugen_oscil_process_general(data, stack, 0);
}
void
ugen_saw_process(void **data, stack_t *stack) /* freq -- output */
{
    ugen_oscil_process_general(data, stack, 1);
}
void
ugen_tri_process(void **data, stack_t *stack) /* freq -- output */
{
    ugen_oscil_process_general(data, stack, 2);
}
void
ugen_pulse_process(void **data, stack_t *stack) /* freq -- output */
{
    ugen_oscil_process_general(data, stack, 3);
}

void
ugen_play_process(void **data, stack_t *stack) /* speed buffer -- output */
{
    value_t value = { VALUE_NUMBER };
    oscil_t *oscil = *data;
    buffer_t *buffer = stack_pop_pointer(stack);
    float speed = stack_pop_number(stack);
    int c;

    if(buffer == NULL || buffer->data == NULL)
        {
            value_t value = { VALUE_ERROR };
            stack_push(stack, value);
            return;
        }

    for(c = 0; c < buffer->chans; ++c)
        {
            value.x.number = buffer->data[(int)(oscil->phase * buffer->len) * buffer->chans + c];
            stack_push(stack, value);
        }

    oscil->phase += speed / audio.rate;
    oscil->phase = fmod(oscil->phase, 1);
    if(oscil->phase < 0)
        oscil->phase += 1;
}
/* */



/* buffer */
void
function_buffer_init(void **data)
{
    *data = calloc(1, sizeof(buffer_t));
}

void
function_buffer_deinit(void **data)
{
    buffer_t *buffer = *data;

    if(buffer->data != NULL)
        free(buffer->data);
}

void
function_buffer_process(void **data, stack_t *stack) /* len chans -- buffer */
{
    value_t value = { VALUE_BUFFER };
    buffer_t *buffer = *data;

    uint8_t chans = stack_pop_number(stack);
    size_t len = stack_pop_number(stack);

    if(len != buffer->len || chans != buffer->chans || buffer->data == NULL)
        {
            buffer->len = len, buffer->chans = chans;
            buffer->data = realloc(buffer->data, len * chans * sizeof(float));
        }

    value.x.pointer = buffer;
    stack_push(stack, value);
}

void
function_buffer_fill_sin(void **data, stack_t *stack) /* buffer -- buffer */
{
    int i, c;
    buffer_t *buffer = stack_peek_pointer(stack);

    if(buffer == NULL || buffer->data == NULL)
        {
            value_t value = { VALUE_ERROR };
            stack_push(stack, value);
            return;
        }

    for(i = 0; i < buffer->len; ++i)
        {
            float x = sin((float) i / (float) buffer->len * TAU);
            for(c = 0; c < buffer->chans; ++c)
                buffer->data[i * buffer->chans + c] = x;
        }

    (void)data;
}

void
function_buffer_fill_noise(void **data, stack_t *stack) /* buffer -- buffer */
{
    int i, c;
    buffer_t *buffer = stack_peek_pointer(stack);

    if(buffer == NULL || buffer->data == NULL)
        {
            value_t value = { VALUE_ERROR };
            stack_push(stack, value);
            return;
        }

    for(i = 0; i < buffer->len; ++i)
        {
            float x = ((float) rand() / (float) RAND_MAX) * 2 - 1;
            for(c = 0; c < buffer->chans; ++c)
                buffer->data[i * buffer->chans + c] = x;
        }

    (void)data;
}
/* */



/* mixing */
void
function_mix(void **data, stack_t *stack) /* left1 right1 left2 right2 -- (left1 + left2) (right1 + right2) */
{
    float left[2], right[2];
    value_t value = { VALUE_NUMBER };
    right[1] = stack_pop_number(stack);
    left[1] = stack_pop_number(stack);
    right[0] = stack_pop_number(stack);
    left[0] = stack_pop_number(stack);

    value.x.number = left[0] + left[1];
    stack_push(stack, value);

    value.x.number = right[0] + right[1];
    stack_push(stack, value);
}
/* */



/* math */
void
function_add(void **data, stack_t *stack) /* b a -- (a + b) */
{
    value_t value = { VALUE_NUMBER };
    float a = stack_pop_number(stack);
    float b = stack_pop_number(stack);

    value.x.number = a + b;
    stack_push(stack, value);

    (void)data;
}

void
function_sub(void **data, stack_t *stack) /* b a -- (b - a) */
{
    value_t value = { VALUE_NUMBER };
    float a = stack_pop_number(stack);
    float b = stack_pop_number(stack);

    value.x.number = b - a;
    stack_push(stack, value);

    (void)data;
}

void
function_mul(void **data, stack_t *stack) /* b a -- (a * b) */
{
    value_t value = { VALUE_NUMBER };
    float a = stack_pop_number(stack);
    float b = stack_pop_number(stack);

    value.x.number = a * b;
    stack_push(stack, value);

    (void)data;
}

void
function_div(void **data, stack_t *stack) /* b a -- (b / a) */
{
    value_t value = { VALUE_NUMBER };
    float a = stack_pop_number(stack);
    float b = stack_pop_number(stack);

    value.x.number = b / a;
    stack_push(stack, value);

    (void)data;
}

void
function_bi2norm(void **data, stack_t *stack) /* bi -- norm */
{
    value_t value = { VALUE_NUMBER };
    float bi = stack_pop_number(stack);

    value.x.number = (bi + 1) * 0.5;
    stack_push(stack, value);

    (void)data;
}

void
function_norm2bi(void **data, stack_t *stack) /* norm -- bi */
{
    value_t value = { VALUE_NUMBER };
    float norm = stack_pop_number(stack);

    value.x.number = norm * 2 - 1;
    stack_push(stack, value);

    (void)data;
}

void
function_constant_general(void **data, stack_t *stack, float constant)
{
    static value_t value = { VALUE_NUMBER };

    value.x.number = constant;
    stack_push(stack, value);

    (void)data;
}

void
function_pi(void **data, stack_t *stack) /* -- pi */
{
    function_constant_general(data, stack, PI);
}

void
function_tau(void **data, stack_t *stack) /* -- tau */
{
    function_constant_general(data, stack, TAU);
}
/* */



/* stack */
void
function_dup(void **data, stack_t *stack)
{
    stack_dup(stack);
    (void)data;
}

void
function_swap(void **data, stack_t *stack)
{
    stack_swap(stack);
    (void)data;
}

void
function_rot(void **data, stack_t *stack)
{
    stack_rot(stack);
    (void)data;
}

void
function_drop(void **data, stack_t *stack)
{
    stack_drop(stack);
    (void)data;
}
/* */



struct _functions
{
    char key[KEY_LENGTH];
    function_t function;
} functions[] =
{
    /* key, { init, deinit, process, process_type } */

    /* ugens */
    {"oscil", { ugen_oscil_init, NULL, ugen_oscil_process, PROCESS_ALWAYS }},
    {"sin", { ugen_oscil_init, NULL, ugen_sin_process, PROCESS_ALWAYS }},
    {"saw", { ugen_oscil_init, NULL, ugen_saw_process, PROCESS_ALWAYS }},
    {"tri", { ugen_oscil_init, NULL, ugen_tri_process, PROCESS_ALWAYS }},
    {"pulse", { ugen_oscil_init, NULL, ugen_pulse_process, PROCESS_ALWAYS }},
    {"play", { ugen_oscil_init, NULL, ugen_play_process, PROCESS_ALWAYS }},

    /* buffer */
    {"buffer", { function_buffer_init, function_buffer_deinit, function_buffer_process, PROCESS_ONCE }},
    {"fill_sin", { NULL, NULL, function_buffer_fill_sin, PROCESS_ONCE }},
    {"fill_noise", { NULL, NULL, function_buffer_fill_noise, PROCESS_ONCE }},

    /* mixing */
    {"mix", { NULL, NULL, function_mix, PROCESS_ALWAYS }},

    /* math */
    {"add", { NULL, NULL, function_add, PROCESS_ALWAYS }},
    {"+", { NULL, NULL, function_add, PROCESS_ALWAYS }},
    {"sub", { NULL, NULL, function_sub, PROCESS_ALWAYS }},
    {"-", { NULL, NULL, function_sub, PROCESS_ALWAYS }},
    {"mul", { NULL, NULL, function_mul, PROCESS_ALWAYS }},
    {"*", { NULL, NULL, function_mul, PROCESS_ALWAYS }},
    {"div", { NULL, NULL, function_div, PROCESS_ALWAYS }},
    {"/", { NULL, NULL, function_div, PROCESS_ALWAYS }},
    {"bi2norm", { NULL, NULL, function_bi2norm, PROCESS_ALWAYS }},
    {"norm2bi", { NULL, NULL, function_norm2bi, PROCESS_ALWAYS }},
    {"pi", { NULL, NULL, function_pi, PROCESS_ALWAYS }},
    {"tau", { NULL, NULL, function_tau, PROCESS_ALWAYS }},

    /* stack */
    {"dup", { NULL, NULL, function_dup, PROCESS_ALWAYS }},
    {"swap", { NULL, NULL, function_swap, PROCESS_ALWAYS }},
    {"rot", { NULL, NULL, function_rot, PROCESS_ALWAYS }},
    {"drop", { NULL, NULL, function_drop, PROCESS_ALWAYS }},
};

static size_t num_functions = sizeof(functions) / sizeof(struct _functions);

function_t *
function_find(char *key)
{
    int i;

    for(i = 0; i < num_functions; ++i)
        if(strcmp(key, functions[i].key) == 0)
            return &functions[i].function;

    return NULL;
}

char *
function_find_key(function_t *function)
{
    int i;

    for(i = 0; i < num_functions; ++i)
        if(function->init == functions[i].function.init &&
                function->deinit == functions[i].function.deinit &&
                function->process == functions[i].function.process)
            return functions[i].key;

    return "UNKNOWN";
}
