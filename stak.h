#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int int16_t;
typedef unsigned short int uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long int int64_t;
typedef unsigned long int uint64_t;

typedef unsigned int uint;

typedef uint8_t bool;

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define SQR(x) ((x) * (x))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define PI 3.14159265358979323846
#define TAU (2 * PI)

#define KEY_LENGTH 16
#define STACK_SIZE 64

#define CHANNELS 2

enum value_type
{
    VALUE_ERROR,
    VALUE_NUMBER,
    VALUE_STRING,
    VALUE_VARIABLE,
    VALUE_BUFFER
};

enum atom_type
{
    ATOM_ERROR,
    ATOM_NUMBER,
    ATOM_STRING,
    ATOM_VARIABLE,
    ATOM_FUNCTION
};

enum process_type
{
    PROCESS_DONE,
    PROCESS_ONCE,
    PROCESS_ALWAYS
};

typedef struct _value value_t;
typedef struct _stack stack_t;
typedef struct _atom atom_t;
typedef struct _procedure procedure_t;
typedef struct _variable variable_t;
typedef struct _variable_ll variable_ll_t;
typedef struct _function function_t;
typedef struct _audio audio_t;
typedef struct _buffer buffer_t;

struct _value
{
    enum value_type type;
    union
    {
        void *pointer;
        float number;
    } x;
};

struct _stack
{
    value_t values[STACK_SIZE];
    uint8_t pos;
};

struct _function
{
    void (*init)(void **data);
    void (*deinit)(void **data);
    void (*process)(void **data, stack_t *stack);
    enum process_type process_type;
    void *data;
};

struct _atom
{
    enum atom_type type;
    union
    {
        function_t function;
        variable_t *variable;
        char *string;
        float number;
    } x;
};

struct _procedure
{
    atom_t *atoms;
    size_t len;
    enum process_type process_type;
};

struct _variable
{
    procedure_t procedure;
    stack_t stack;
    char key[KEY_LENGTH];
};

struct _variable_ll
{
    variable_t *variable;
    variable_ll_t *next;
};

struct _audio
{
    float rate;
    uint8_t chans;
};

struct _buffer
{
    float *data;
    size_t len;
    uint8_t chans;
};

/* stak.c */
extern bool reload;


/* stack.c */
void
value_print(value_t *value);

void
stack_print(stack_t *stack);

void
stack_push(stack_t *stack, value_t value);

value_t
stack_pop(stack_t *stack);

float
stack_pop_number(stack_t *stack);

void *
stack_pop_pointer(stack_t *stack);

value_t *
stack_peek(stack_t *stack);

float
stack_peek_number(stack_t *stack);

void *
stack_peek_pointer(stack_t *stack);

void
stack_concat(stack_t *dest, stack_t *source);

void
stack_reset(stack_t *stack);

void
stack_dup(stack_t *stack);

void
stack_swap(stack_t *stack);

void
stack_rot(stack_t *stack);

void
stack_drop(stack_t *stack);
/* */



/* variable.c */
extern variable_ll_t variables[2];

void
variable_print(variable_t *variable);

void
variable_deinit(variable_t *variable);

void
variable_process(variable_t *variable);

void
variable_reset_process_type(variable_t *variable);



void
variable_ll_print(variable_ll_t *root);

void
variable_ll_init(variable_ll_t *root);

void
variable_ll_free(variable_ll_t *root);

void
variable_ll_deinit(variable_ll_t *root);

variable_ll_t *
variable_ll_add(variable_ll_t *root, char *key);

void
variable_ll_remove(variable_ll_t *root, char *key);

variable_t *
variable_ll_find(variable_ll_t *root, char *key);

variable_ll_t *
variable_ll_find_ll(variable_ll_t *root, char *key);

void
variable_ll_merge(variable_ll_t *old, variable_ll_t *new);

void
variable_ll_process(variable_ll_t *root);
/* */



/* procedure.c */
void
procedure_print(procedure_t *procedure);

void
procedure_deinit(procedure_t *procedure);

void
procedure_push(procedure_t *procedure, atom_t atom);

void
procedure_process(procedure_t *procedure, stack_t *stack);

void
procedure_reset_process_type(procedure_t *procedure);
/* */



/* atom.c */
void
atom_print(atom_t *atom);

void
atom_init(atom_t *atom);

void
atom_deinit(atom_t *atom);

void
atom_process(atom_t *atom, stack_t *stack);
/* */



/* parser.c */
typedef struct _token token_t;

void
parser_file(char *filename);

void
parser_lex(char *text, size_t len);

void
parser_parse(token_t *tokens, size_t num_tokens);
/* */



/* audio.c */
extern audio_t audio;

int
audio_init();

void
audio_deinit();
/* */



/* function.c */
function_t *
function_find(char *key);

char *
function_find_key(function_t *function);
/* */
