/* TODO:
 * - remove 'out' as a default variable in parser, set it explicitly like other variables.
 *   this prevents you accidentally editting the output when using the repl
 * */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

typedef unsigned int uint;

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define SQR(x) ((x) * (x))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define PI 3.14159265358979323846
#define TAU (2 * PI)

#define PROFILE(body) {\
        clock_t start, end;\
        double t;\
        start = clock();\
body\
        end = clock();\
        t = (end - start) / (double)CLOCKS_PER_SEC;\
        printf("TIME TAKEN %f\n", t);\
	}

/* enums */
enum status
{
    OK,
    BAD
};

enum atom_type
{
    ATOM_ERROR,
    ATOM_FLOAT,
    ATOM_STRING,
    ATOM_FUNCTION,
    ATOM_UGEN,
    ATOM_VARIABLE
};
extern const char *atom_type_name[];

enum value_type
{
    VALUE_ERROR,
    VALUE_FLOAT,
    VALUE_STRING,
    VALUE_BUFFER
};
extern const char *value_type_name[];

/* structs */
struct value
{
    enum value_type type;
    union
    {
        float f;
        void *p;
    } x;
};

#define MAX_STACK_SIZE 64
struct stack
{
    struct value data[MAX_STACK_SIZE];
    uint pos;
};

struct procedure
{
    struct atom *atoms;
    size_t num;
};

struct variable
{
    struct procedure proc;
    struct stack stack;
    bool dynamic;
};

#define MAX_VARIABLE_LENGTH 16
struct variable_ll
{
    char key[MAX_VARIABLE_LENGTH];
    struct variable_ll *next;
    struct variable v;
};

struct ugen
{
    enum status (*init)(struct ugen *);
    void (*deinit)(struct ugen *);
    enum status (*update)(struct stack *s, struct ugen *u);
    void *data;
};

typedef enum status (*function)(struct stack *s);

struct atom
{
    enum atom_type type;
    union
    {
        float f;
        char *s;
        function fn;
        struct ugen u;
        struct
        {
            char key[MAX_VARIABLE_LENGTH];
            struct variable *v;
        } v;
    } x;
};

struct audio
{
    float rate;
};

extern bool reload;
extern struct variable_ll new_root;

void print_atom(struct atom *a);

/* stack.c */
enum status duplicate(struct stack *s);
struct value pop(struct stack *s);
float popf(struct stack *s);
struct value peek(struct stack *s, int offset);
enum status push(struct stack *s, struct value v);
enum status take(struct stack *src, struct stack *dest, int num);
enum status concat(struct stack *src, struct stack *dest);
bool check(struct stack *s, enum value_type type, int offset);
void print_stack(struct stack *s);

/* parse.c */
struct token;
void parse_file(char *file);
void lexer(char *str, size_t len);
void parser(struct token *tokens, size_t num_tokens);

/* var.c */
extern struct variable_ll stak_root;
void deinit_var_ll(struct variable_ll *root);
void deinit_vars(struct variable_ll *root);
void deinit_proc(struct procedure *p);
void deinit_var(struct variable *v);
void print_vars(struct variable_ll *root);
char *get_var_name(struct variable_ll *root, struct variable *v);
struct variable *find_var(struct variable_ll *root, char *key);
struct variable *add_var(struct variable_ll *root, char *key);
bool remove_var(struct variable_ll *root, char *key);
void append_atom(struct variable *v, struct atom *a);
void process_var(struct variable *v);
void process_vars(struct variable_ll *root);
void replace_vars(struct variable_ll *root, struct variable_ll *new_root);
void reset_var_pointers(struct variable_ll *root);

/* ugen.c */
void print_ugens();
char *get_ugen_name(struct ugen *u);
struct ugen *find_ugen(char *key);

/* func.c */
void print_funcs();
char *get_func_name(function fn);
function find_func(char *key);

/* audio.c */
extern struct audio audio;
enum status init_audio();
void deinit_audio();

/* udp.c */
extern bool server_running;
enum status server_callback(char *str, size_t len);
enum status run_server();

/* util.c */
void init_notes();
float midi2freq(int m);
int freq2midi(float freq);
