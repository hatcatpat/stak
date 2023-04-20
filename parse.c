#include "stak.h"

enum lexer_types
{
    LEX_SPACE,
    LEX_BREAK,
    LEX_COMMENT,
    LEX_NUM,
    LEX_SYM,
    LEX_STRING,
    LEX_DEF
};
const char *lexer_type_names[] =
{
    "SPACE",
    "BREAK",
    "COMMENT",
    "NUM",
    "SYM",
    "STRING",
    "DEF"
};

struct token
{
    char *str;
    enum lexer_types type;
};

void
parse_file(char *file)
{
    char *str;
    size_t len;

    FILE *f = fopen(file, "r");
    if(f == NULL)
        return;

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    str = malloc(len);
    fread(str, 1, len, f);
    fclose(f);

    lexer(str, len);

    free(str);
}

void
lexer(char *str, size_t len)
{
    char c;
    bool break_flag = 0;
    int i = 0, start = 0;
    enum lexer_types type = LEX_SPACE;

    struct token *tokens = NULL, *token = NULL;
    size_t num_tokens = 0;

    while(i < len)
        {
            c = str[i];

            switch(c)
                {
                    case '[':
                    case '(':
                    case ']':
                    case ')':
                    case '\t':
                        c = ' ';
                        break;
                }

            switch(type)
                {
                    case LEX_BREAK:
                        switch(c)
                            {
                                case ' ':
                                    break;

                                case '#':
                                    type = LEX_COMMENT;
                                case '\n':
                                    break_flag = true;
                                    break;

                                default:
                                    if(break_flag)
                                        {
                                            num_tokens++;
                                            tokens = realloc(tokens, num_tokens * sizeof(struct token));
                                            memset(&tokens[num_tokens - 1], 0, sizeof(struct token));
                                            tokens[num_tokens - 1].type = LEX_BREAK;
                                            break_flag = false;
                                        }
                                    type = LEX_SPACE;
                                    break;
                            }
                    case LEX_SPACE:
                        switch(c)
                            {
                                case ' ':
                                    break;

                                case '\n':
                                    type = LEX_BREAK;
                                    break;

                                case '#':
                                    type = LEX_COMMENT;
                                    break;

                                case '\"':
                                    type = LEX_STRING;
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
                                    type = LEX_NUM;
                                    start = i;
                                    break;

                                default:
                                    type = LEX_SYM;
                                    start = i;
                                    break;
                            }
                        break;


                    case LEX_COMMENT:
                        if(c == '\n')
                            type = LEX_BREAK;
                        break;


                    case LEX_STRING:
                        if(c == '\"')
                            {
                                num_tokens++;
                                tokens = realloc(tokens, num_tokens * sizeof(struct token));
                                token = &tokens[num_tokens - 1];
                                memset(token, 0, sizeof(struct token));

                                token->type = LEX_STRING;

                                if(i != start)
                                    {
                                        size_t len = i - start;
                                        token->str = malloc(len + 1);
                                        strncpy(token->str, &str[start], len);
                                        token->str[len] = '\0';
                                    }

                                type = LEX_SPACE;
                            }
                        break;


                    default:
                        switch(c)
                            {
                                case ':':
                                    type = LEX_DEF;
                                case '#':
                                case ' ':
                                case '\n':
                                {
                                    if(i != start)
                                        {
                                            size_t len = i - start;

                                            num_tokens++;
                                            tokens = realloc(tokens, num_tokens * sizeof(struct token));
                                            token = &tokens[num_tokens - 1];

                                            token->str = malloc(len + 1);
                                            strncpy(token->str, &str[start], len);
                                            token->str[len] = '\0';

                                            token->type = type;
                                        }

                                    if(c == '#')
                                        type = LEX_COMMENT;
                                    else if (c == '\n')
                                        type = LEX_BREAK;
                                    else
                                        type = LEX_SPACE;
                                }
                                break;
                            }
                        break;
                }

            i++;
        }

    printf("tokens...\n");

    for(i = 0; i < num_tokens; ++i)
        if(tokens[i].str)
            printf("[%s] %s\n", lexer_type_names[tokens[i].type], tokens[i].str);
        else
            printf("[%s]\n", lexer_type_names[tokens[i].type]);

    parser(tokens, num_tokens);
}

void
parser(struct token *tokens, size_t num_tokens)
{
    struct token *token = NULL;
    struct variable_ll root = {"out", NULL};
    struct variable *var = &root.v;
    int i = 0;
    struct atom atom;

    if(tokens == NULL)
        return;

    while(i < num_tokens)
        {
            token = &tokens[i++];

            if(token->type == LEX_DEF)
                if(find_var(&root, token->str) == NULL)
                    add_var(&root, token->str);
        }

    i = 0;
    while(i < num_tokens)
        {
            token = &tokens[i++];

            switch(token->type)
                {
                    case LEX_NUM:
                        atom.type = ATOM_FLOAT;
                        atom.x.f = atof(token->str);
                        append_atom(var, &atom);
                        break;

                    case LEX_SYM:
                    {
                        union
                        {
                            struct ugen *u;
                            function fn;
                        } x;

                        if((x.u = find_ugen(token->str)))
                            {
                                atom.type = ATOM_UGEN;
                                atom.x.u = *x.u;
                                atom.x.u.init(&atom.x.u);
                            }
                        else if((x.fn = find_func(token->str)))
                            {
                                atom.type = ATOM_FUNCTION;
                                atom.x.fn = x.fn;
                            }
                        else if(find_var(&root, token->str) || find_var(&stak_root, token->str))
                            {
                                atom.type = ATOM_VARIABLE;
                                atom.x.v.v = NULL;
                                strcpy(atom.x.v.key, token->str);
                            }
                        else
                            atom.type = ATOM_ERROR;
                    }

                    append_atom(var, &atom);
                    break;

                    case LEX_STRING:
                        atom.type = ATOM_STRING;
                        atom.x.s = token->str;
                        append_atom(var, &atom);
                        break;

                    case LEX_DEF:
                        if(strcmp(token->str, "out") == 0)
                            var = &root.v;
                        else
                            var = find_var(&root, token->str);
                        break;

                    case LEX_BREAK:
                        var = &root.v;
                        break;

                    default:
                        break;
                }
        }

    for(i = 0; i < num_tokens; ++i)
        if(tokens[i].type != LEX_STRING)
            free(tokens[i].str);
    free(tokens);

    printf("\nparsed vars:\n");
    print_vars(&root);

    while(reload)
        {
        }

    new_root = root;
    reload = true;
}

