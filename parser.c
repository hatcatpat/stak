#include "stak.h"

enum token_type
{
    TOKEN_SPACE,
    TOKEN_COMMENT,
    TOKEN_NUMBER,
    TOKEN_SYMBOL,
    TOKEN_STRING,
    TOKEN_VARIABLE
};

const char *token_type_names[] =
{
    "SPACE",
    "COMMENT",
    "NUMBER",
    "SYMBOL",
    "STRING",
    "VARIABLE"
};

struct _token
{
    enum token_type type;
    char *string;
};

void
parser_file(char *file)
{
    char *string;
    size_t len;

    FILE *f = fopen(file, "r");
    if(f == NULL)
        return;

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    string = malloc(len);
    fread(string, 1, len, f);
    fclose(f);

    parser_lex(string, len);

    free(string);
}

void
parser_string(char *string)
{
    parser_lex(string, strlen(string));
}

void
parser_lex(char *string, size_t len)
{
    char c;
    int i = 0, start = 0;
    enum token_type type = TOKEN_SPACE;

    token_t *tokens = NULL, *token = NULL;
    size_t num_tokens = 0;

    while(i < len)
        {
            c = string[i];

            switch(c)
                {
                    case '[':
                    case '(':
                    case ']':
                    case ')':
                    case '\t':
                    case '\n':
                        c = ' ';
                        break;
                }

            switch(type)
                {
                    case TOKEN_SPACE:
                        switch(c)
                            {
                                case ' ':
                                    break;

                                case '#':
                                    type = TOKEN_COMMENT;
                                    break;

                                case '\"':
                                    type = TOKEN_STRING;
                                    start = i + 1;
                                    break;

                                case '0':
                                case '1':
                                case '2':
                                case '3':
                                case '4':
                                case '5':
                                case '6':
                                case '7':
                                case '8':
                                case '9':
                                case '-':
                                case '.':
                                    type = TOKEN_NUMBER;
                                    start = i;
                                    break;

                                default:
                                    type = TOKEN_SYMBOL;
                                    start = i;
                                    break;
                            }
                        break;

                    case TOKEN_COMMENT:
                        if(string[i] == '\n')
                            type = TOKEN_SPACE;
                        break;

                    case TOKEN_STRING:
                        if(c == '\"')
                            {
                                num_tokens++;
                                tokens = realloc(tokens, num_tokens * sizeof(token_t));
                                token = &tokens[num_tokens - 1];
                                memset(token, 0, sizeof(token_t));

                                token->type = TOKEN_STRING;

                                if(i != start)
                                    {
                                        size_t len = i - start;
                                        token->string = malloc(len + 1);
                                        strncpy(token->string, &string[start], len);
                                        token->string[len] = '\0';
                                    }

                                type = TOKEN_SPACE;
                            }
                        break;

                    case TOKEN_NUMBER:
                        switch(c)
                            {
                                case '0':
                                case '1':
                                case '2':
                                case '3':
                                case '4':
                                case '5':
                                case '6':
                                case '7':
                                case '8':
                                case '9':
                                case '.':
                                    break;

                                case ':':
                                case '#':
                                case ' ':
                                    break;

                                default:
                                    type = TOKEN_SYMBOL;
                                    break;
                            }
                    /* FALLTHROUGH */
                    default:
                        switch(c)
                            {
                                case ':':
                                    type = TOKEN_VARIABLE;
                                /* FALLTHROUGH */
                                case '#':
                                case ' ':
                                {
                                    if(i != start)
                                        {
                                            size_t len = i - start;

                                            num_tokens++;
                                            tokens = realloc(tokens, num_tokens * sizeof(token_t));
                                            token = &tokens[num_tokens - 1];

                                            token->string = malloc(len + 1);
                                            strncpy(token->string, &string[start], len);
                                            token->string[len] = '\0';

                                            if(type == TOKEN_NUMBER && len == 1 && token->string[0] == '-')
                                                token->type = TOKEN_SYMBOL;
                                            else
                                                token->type = type;
                                        }

                                    if(c == '#')
                                        type = TOKEN_COMMENT;
                                    else
                                        type = TOKEN_SPACE;
                                }
                                break;
                            }
                        break;
                }

            i++;
        }

    printf("[tokens]\n");
    for(i = 0; i < num_tokens; ++i)
        printf("[%i] %s %s\n", i, token_type_names[tokens[i].type], tokens[i].string);

    parser_parse(tokens, num_tokens);
}

void
parser_parse(token_t *tokens, size_t num_tokens)
{
    token_t *token = NULL;
    variable_t *variable = NULL;
    variable_ll_t root;
    atom_t atom;
    int i;

    if(tokens == NULL)
        return;

    variable_ll_init(&root);

    i = 0;
    while(i < num_tokens)
        {
            token = &tokens[i++];

            if(token->type == TOKEN_VARIABLE)
                if(variable_ll_find(&root, token->string) == NULL)
                    variable_ll_add(&root, token->string);
        }

    i = 0;
    while(i < num_tokens)
        {
            token = &tokens[i++];

            if(token->type == TOKEN_VARIABLE)
                {
                    variable = variable_ll_find(&root, token->string);
                    continue;
                }

            if(variable == NULL)
                continue;

            switch(token->type)
                {
                    case TOKEN_NUMBER:
                        atom.type = ATOM_NUMBER;
                        atom.x.number = atof(token->string);
                        procedure_push(&variable->procedure, atom);
                        break;

                    case TOKEN_SYMBOL:
                    {
                        function_t *function;

                        if((function = function_find(token->string)) != NULL)
                            {
                                atom.type = ATOM_FUNCTION;
                                atom.x.function = *function;
                                atom_init(&atom);
                            }
                        else if(variable_ll_find(&root, token->string) != NULL ||
                                variable_ll_find(&variables[0], token->string) != NULL ||
                                variable_ll_find(&variables[1], token->string) != NULL)
                            {
                                atom.type = ATOM_VARIABLE;
                                atom.x.variable_key.variable = NULL;
                                strncpy(atom.x.variable_key.key, token->string, KEY_LENGTH);
                            }
                        else
                            atom.type = ATOM_ERROR;

                        procedure_push(&variable->procedure, atom);
                    }
                    break;

                    case TOKEN_STRING:
                        atom.type = ATOM_STRING;
                        atom.x.string = token->string;

                        procedure_push(&variable->procedure, atom);
                        break;

                    default:
                        break;
                }
        }

    for(i = 0; i < num_tokens; ++i)
        if(tokens[i].type != TOKEN_STRING)
            free(tokens[i].string);
    free(tokens);

    printf("\nparsed variables...\n");
    variable_ll_print(&root);

    while(reload)
        {
        }

    variables[1] = root;
    reload = 1;
}

void
parser_command(char *string)
{
    if(strcmp(string, "q\n") == 0)
        running = 0;
    else if(strcmp(string, "r\n") == 0)
        parser_file("example.st");
    else if(strcmp(string, "\n") == 0)
        variable_ll_print(&variables[0]);
    else
        parser_string(string);
}
