#include "stak.h"

#include <math.h>



/* utils */
float
map(float x, float in_low, float in_high, float out_low, float out_high)
{
    return (x - in_low) * (out_high - out_low) / (in_high - in_low) + out_low;
}

float
random_float(float low, float high)
{
    return ((float)rand() / (float)RAND_MAX) * (high - low) + low;
}

float
bi2norm(float bi)
{
    return (bi + 1) * 0.5;
}

float
norm2bi(float norm)
{
    return norm * 2 - 1;
}

bool
gate(float x)
{
    return x >= 0.5;
}

float
recip(float x)
{
    return x == 0 ? 0 : 1 / x;
}

float
neg(float x)
{
    return -x;
}

float
sec2samp(float sec)
{
    return sec * audio.rate;
}

float
samp2sec(float samp)
{
    return samp / audio.rate;
}
/* */



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
    return norm2bi(phase / TAU);
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
ugen_play_process(void **data, stack_t *stack) /* speed buffer -- c0 c1 ... cn */
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
            value.x.number = buffer->data[(int)oscil->phase * buffer->chans + c];
            stack_push(stack, value);
        }

    oscil->phase += speed;
    oscil->phase = fmod(oscil->phase, buffer->len);
    if(oscil->phase < 0)
        oscil->phase += buffer->len;
}

void
ugen_record_init(void **data)
{
    *data = calloc(1, sizeof(uint_t));
}

void
ugen_record_process(void **data, stack_t *stack) /* c0 c1 ... cn record buffer -- */
{
    uint_t *pos = *data;
    buffer_t *buffer = stack_pop_pointer(stack);
    bool record = gate(stack_pop_number(stack));
    int c;

    if(buffer == NULL || buffer->data == NULL)
        {
            value_t value = { VALUE_ERROR };
            stack_push(stack, value);
            return;
        }

    if(record)
        for(c = 0; c < buffer->chans; ++c)
            buffer->data[(*pos % buffer->len) * buffer->chans + (buffer->chans - 1 - c)] = stack_pop_number(stack);

    (*pos)++;
}

typedef struct _rand
{
    float phase, value;
} rand_t;

void
ugen_rand_init(void **data)
{
    rand_t *randy = calloc(1, sizeof(rand_t));
    randy->value = random_float(-1, 1);

    *data = randy;
}

void
ugen_rand_process(void **data, stack_t *stack) /* freq -- output */
{
    value_t value = { VALUE_NUMBER };
    rand_t *randy = *data;
    float freq = stack_pop_number(stack);

    randy->phase += ABS(freq) / audio.rate;
    if(randy->phase > 1)
        randy->value = random_float(-1, 1);
    randy->phase = fmod(randy->phase, 1);

    value.x.number = randy->value;
    stack_push(stack, value);

}

void
ugen_latch_init(void **data)
{
    *data = calloc(1, sizeof(float));
}

void
ugen_latch_process(void **data, stack_t *stack) /* input toggle -- output */
{
    value_t value = { VALUE_NUMBER };
    float *current = *data;
    bool active = gate(stack_pop_number(stack));
    float input = stack_pop_number(stack);

    if(active)
        *current = input;

    value.x.number = *current;
    stack_push(stack, value);
}

#define DELAY_LENGTH (44100 * 4)
typedef struct _delay
{
    float data[DELAY_LENGTH];
    uint_t pos;
} delay_t;

void
ugen_delay_init(void **data)
{
    *data = calloc(1, sizeof(delay_t));
}

void
ugen_delay_process(void **data, stack_t *stack) /* input time feedback -- output */
{
    value_t value = { VALUE_NUMBER };
    delay_t *delay = *data;
    float feedback = stack_pop_number(stack);
    int time_samp = stack_pop_number(stack) * audio.rate;
    float input = stack_pop_number(stack);

    if(delay == NULL)
        {
            value.type = VALUE_ERROR;
            stack_push(stack, value);
            return;
        }
    else
        {
            uint_t read;

            time_samp = MIN(ABS(time_samp), DELAY_LENGTH);
            read = (delay->pos + (DELAY_LENGTH - time_samp)) % DELAY_LENGTH;

            /* read */
            value.x.number = delay->data[read];
            stack_push(stack, value);

            /* write */
            delay->data[delay->pos++] = input + value.x.number * feedback;

            if(delay->pos >= DELAY_LENGTH)
                delay->pos = 0;
        }
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
function_buffer_info(void **data, stack_t *stack) /* buffer -- len chans */
{
    value_t value = { VALUE_NUMBER };
    buffer_t *buffer = stack_pop_pointer(stack);

    if(buffer == NULL)
        {
            value.x.number = 0;
            stack_push(stack, value);
            stack_push(stack, value);

            return;
        }

    value.x.number = buffer->len;
    stack_push(stack, value);

    value.x.number = buffer->chans;
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
}

void
function_file_process(void **data, stack_t *stack) /* file -- buffer */
{
    value_t value = { VALUE_BUFFER };
    buffer_t *buffer = *data;
    char *file = stack_pop_pointer(stack);

    if(buffer->data != NULL)
        free(buffer->data);

    if(file == NULL)
        return;

    *buffer = wav_decode(file);
    value.x.pointer = buffer;
    stack_push(stack, value);
}
/* */



/* audio */
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

void
function_in(void **data, stack_t *stack) /* -- in_0 in_1 ... in_n */
{
    value_t value = { VALUE_NUMBER };
    uint8_t c;

    for(c = 0; c < CHANNELS; ++c)
        {
            value.x.number = audio.in[c];
            stack_push(stack, value);
        }
}

void
function_out(void **data, stack_t *stack) /* out_0 out_1 ... out_n -- */
{
    uint8_t c;

    for(c = 0; c < CHANNELS; ++c)
        audio.out[CHANNELS - 1 - c] += stack_pop_number(stack);
}

void
function_pan(void **data, stack_t *stack) /* mono pan[-1,1] -- stereo */
{
    value_t value = { VALUE_NUMBER };
    float angle = bi2norm(stack_pop_number(stack));
    float input = stack_pop_number(stack);

    if(angle < 0)
        angle = 0;
    else if(angle > 1)
        angle = 1;

    angle *= PI * 0.5;

    value.x.number = input * cos(angle);
    stack_push(stack, value);

    value.x.number = input * sin(angle);
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
}

void
function_sub(void **data, stack_t *stack) /* b a -- (b - a) */
{
    value_t value = { VALUE_NUMBER };
    float a = stack_pop_number(stack);
    float b = stack_pop_number(stack);

    value.x.number = b - a;
    stack_push(stack, value);
}

void
function_mul(void **data, stack_t *stack) /* b a -- (a * b) */
{
    value_t value = { VALUE_NUMBER };
    float a = stack_pop_number(stack);
    float b = stack_pop_number(stack);

    value.x.number = a * b;
    stack_push(stack, value);
}

void
function_div(void **data, stack_t *stack) /* b a -- (b / a) */
{
    value_t value = { VALUE_NUMBER };
    float a = stack_pop_number(stack);
    float b = stack_pop_number(stack);

    value.x.number = b / a;
    stack_push(stack, value);
}

void
function_recip(void **data, stack_t *stack) /* a -- (1 / a) */
{
    value_t value = { VALUE_NUMBER };

    value.x.number = stack_pop_number(stack);
    if(value.x.number != 0)
        value.x.number = 1 / value.x.number;
    stack_push(stack, value);
}

void
function_neg(void **data, stack_t *stack) /* a -- -a */
{
    value_t value = { VALUE_NUMBER };

    value.x.number = -stack_pop_number(stack);
    stack_push(stack, value);
}

void
function_bi2norm(void **data, stack_t *stack) /* bi -- norm */
{
    value_t value = { VALUE_NUMBER };

    value.x.number = bi2norm(stack_pop_number(stack));
    stack_push(stack, value);
}

void
function_norm2bi(void **data, stack_t *stack) /* norm -- bi */
{
    value_t value = { VALUE_NUMBER };

    value.x.number = norm2bi(stack_pop_number(stack));
    stack_push(stack, value);
}

void
function_sec2samp(void **data, stack_t *stack) /* seconds -- samples */
{
    value_t value = { VALUE_NUMBER };

    value.x.number = stack_pop_number(stack) * audio.rate;
    stack_push(stack, value);
}

void
function_samp2sec(void **data, stack_t *stack) /* samples -- seconds */
{
    value_t value = { VALUE_NUMBER };

    value.x.number = stack_pop_number(stack) / audio.rate;
    stack_push(stack, value);
}
/* */



/* math multi-channel (x0 x1 ... xn) (y0 y1 ... yn) n -- (z0 z1 ... zn) */
#define MAX_CHANNELS 16

void
multichannel_unary(void **data, stack_t *stack, float (*unary)(float x))
{
    value_t value = { VALUE_NUMBER };
    float temp[MAX_CHANNELS];
    uint_t i;
    uint_t n = stack_pop_number(stack);

    n = MIN(n, MAX_CHANNELS);

    for(i = 0; i < n; ++i)
        temp[n - 1 - i] = stack_pop_number(stack);

    for(i = 0; i < n; ++i)
        {
            value.x.number = unary(temp[i]);
            stack_push(stack, value);
        }
}

void
multichannel_binop(void **data, stack_t *stack, float (*binop)(float a, float b)) /* b a -- (a + b) */
{
    value_t value = { VALUE_NUMBER };
    float temp[MAX_CHANNELS];
    uint_t i;
    uint_t n = stack_pop_number(stack);

    n = MIN(n, MAX_CHANNELS);

    for(i = 0; i < n; ++i)
        temp[n - 1 - i] = stack_pop_number(stack);

    for(i = 0; i < n; ++i)
        temp[n - 1 - i] = binop(stack_pop_number(stack), temp[n - 1 - i]);

    for(i = 0; i < n; ++i)
        {
            value.x.number = temp[i];
            stack_push(stack, value);
        }
}

float
binop_add(float a, float b)
{
    return a + b;
}
void
function_add_(void **data, stack_t *stack) /* b a -- (a + b) */
{
    multichannel_binop(data, stack, binop_add);
}

float
binop_sub(float a, float b)
{
    return a - b;
}
void
function_sub_(void **data, stack_t *stack) /* b a -- (b - a) */
{
    multichannel_binop(data, stack, binop_sub);
}

float
binop_mul(float a, float b)
{
    return a * b;
}
void
function_mul_(void **data, stack_t *stack) /* b a -- (b * a) */
{
    multichannel_binop(data, stack, binop_mul);
}

void
function_scale_(void **data, stack_t *stack) /* (x0 x1 x2 ... xn) s n -- x0*s x1*s x2*s ... xn*s */
{
    value_t value = { VALUE_NUMBER };
    float temp[MAX_CHANNELS];
    uint_t i;
    uint_t n = stack_pop_number(stack);
    float scale = stack_pop_number(stack);

    n = MIN(n, MAX_CHANNELS);

    for(i = 0; i < n; ++i)
        temp[n - 1 - i] = stack_pop_number(stack);

    for(i = 0; i < n; ++i)
        {
            value.x.number = temp[i] * scale;
            stack_push(stack, value);
        }
}

float
binop_div(float a, float b)
{
    return b == 0 ? 0 : a / b;
}
void
function_div_(void **data, stack_t *stack) /* b a -- (b / a) */
{
    multichannel_binop(data, stack, binop_div);
}

void
function_recip_(void **data, stack_t *stack) /* a -- (1 / a) */
{
    multichannel_unary(data, stack, recip);
}

void
function_neg_(void **data, stack_t *stack) /* a -- -a */
{
    multichannel_unary(data, stack, neg);
}

void
function_bi2norm_(void **data, stack_t *stack) /* bi -- norm */
{
    multichannel_unary(data, stack, bi2norm);
}

void
function_norm2bi_(void **data, stack_t *stack) /* norm -- bi */
{
    multichannel_unary(data, stack, norm2bi);
}

void
function_sec2samp_(void **data, stack_t *stack) /* seconds -- samples */
{
    multichannel_unary(data, stack, sec2samp);
}

void
function_samp2sec_(void **data, stack_t *stack) /* samples -- seconds */
{
    multichannel_unary(data, stack, samp2sec);
}
/* */


/* constants */
void
function_constant_general(void **data, stack_t *stack, float constant)
{
    value_t value = { VALUE_NUMBER };

    value.x.number = constant;
    stack_push(stack, value);
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

void
function_rate(void **data, stack_t *stack) /* -- audio.rate */
{
    function_constant_general(data, stack, audio.rate);
}
/* */



/* stack */
void
function_dup(void **data, stack_t *stack)
{
    stack_dup(stack);
}

void
function_swap(void **data, stack_t *stack)
{
    stack_swap(stack);
}

void
function_rot(void **data, stack_t *stack)
{
    stack_rot(stack);
}

void
function_drop(void **data, stack_t *stack)
{
    stack_drop(stack);
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
    {"record", { ugen_record_init, NULL, ugen_record_process, PROCESS_ALWAYS }},
    {"rand", { ugen_rand_init, NULL, ugen_rand_process, PROCESS_ALWAYS }},
    {"latch", { ugen_latch_init, NULL, ugen_latch_process, PROCESS_ALWAYS }},
    {"delay", { ugen_delay_init, NULL, ugen_delay_process, PROCESS_ALWAYS }},

    /* buffer */
    {"buffer", { function_buffer_init, function_buffer_deinit, function_buffer_process, PROCESS_ONCE }},
    {"buffer_info", { NULL, NULL, function_buffer_info, PROCESS_WHENEVER }},
    {"fill_sin", { NULL, NULL, function_buffer_fill_sin, PROCESS_ONCE }},
    {"fill_noise", { NULL, NULL, function_buffer_fill_noise, PROCESS_ONCE }},
    {"file", { function_buffer_init, function_buffer_deinit, function_file_process, PROCESS_ONCE }},

    /* audio */
    {"mix", { NULL, NULL, function_mix, PROCESS_ALWAYS }},
    {"in", { NULL, NULL, function_in, PROCESS_ALWAYS }},
    {"out", { NULL, NULL, function_out, PROCESS_ALWAYS }},
    {"pan", { NULL, NULL, function_pan, PROCESS_ALWAYS }},

    /* math */
    {"add", { NULL, NULL, function_add, PROCESS_WHENEVER }},
    {"+", { NULL, NULL, function_add, PROCESS_WHENEVER }},
    {"sub", { NULL, NULL, function_sub, PROCESS_WHENEVER }},
    {"-", { NULL, NULL, function_sub, PROCESS_WHENEVER }},
    {"mul", { NULL, NULL, function_mul, PROCESS_WHENEVER }},
    {"*", { NULL, NULL, function_mul, PROCESS_WHENEVER }},
    {"div", { NULL, NULL, function_div, PROCESS_WHENEVER }},
    {"/", { NULL, NULL, function_div, PROCESS_WHENEVER }},
    {"recip", { NULL, NULL, function_recip, PROCESS_WHENEVER }},
    {"neg", { NULL, NULL, function_neg, PROCESS_WHENEVER }},
    {"bi2norm", { NULL, NULL, function_bi2norm, PROCESS_WHENEVER }},
    {"norm2bi", { NULL, NULL, function_norm2bi, PROCESS_WHENEVER }},
    {"sec2samp", { NULL, NULL, function_sec2samp, PROCESS_WHENEVER }},
    {"samp2sec", { NULL, NULL, function_samp2sec, PROCESS_WHENEVER }},

    /* math multi-channel */
    {"add_", { NULL, NULL, function_add_, PROCESS_WHENEVER }},
    {"+_", { NULL, NULL, function_add_, PROCESS_WHENEVER }},
    {"sub_", { NULL, NULL, function_sub_, PROCESS_WHENEVER }},
    {"-_", { NULL, NULL, function_sub_, PROCESS_WHENEVER }},
    {"mul_", { NULL, NULL, function_mul_, PROCESS_WHENEVER }},
    {"*_", { NULL, NULL, function_mul_, PROCESS_WHENEVER }},
    {"scale_", { NULL, NULL, function_scale_, PROCESS_WHENEVER }},
    {"div_", { NULL, NULL, function_div_, PROCESS_WHENEVER }},
    {"/_", { NULL, NULL, function_div_, PROCESS_WHENEVER }},
    {"recip_", { NULL, NULL, function_recip_, PROCESS_WHENEVER }},
    {"neg_", { NULL, NULL, function_neg_, PROCESS_WHENEVER }},
    {"bi2norm_", { NULL, NULL, function_bi2norm_, PROCESS_WHENEVER }},
    {"norm2bi_", { NULL, NULL, function_norm2bi_, PROCESS_WHENEVER }},
    {"sec2samp_", { NULL, NULL, function_sec2samp_, PROCESS_WHENEVER }},
    {"samp2sec_", { NULL, NULL, function_samp2sec_, PROCESS_WHENEVER }},

    /* constants */
    {"pi", { NULL, NULL, function_pi, PROCESS_WHENEVER }},
    {"tau", { NULL, NULL, function_tau, PROCESS_WHENEVER }},
    {"rate", { NULL, NULL, function_rate, PROCESS_WHENEVER }},

    /* stack */
    {"dup", { NULL, NULL, function_dup, PROCESS_WHENEVER }},
    {"swap", { NULL, NULL, function_swap, PROCESS_WHENEVER }},
    {"rot", { NULL, NULL, function_rot, PROCESS_WHENEVER }},
    {"drop", { NULL, NULL, function_drop, PROCESS_WHENEVER }},
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
