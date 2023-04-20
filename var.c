#include "stak.h"

struct variable_ll stak_root = {"out", NULL};

void
deinit_proc(struct procedure *p)
{
    int i;
    struct atom *a = NULL;

    for(i = 0; i < p->num; ++i)
        {
            a = &p->atoms[i];

            switch(a->type)
                {
                    case ATOM_UGEN:
                        a->x.u.deinit(&a->x.u);
                        break;

                    case ATOM_STRING:
                        free(a->x.s);
                        break;

                    default:
                        break;
                }
        }

    if(p->atoms)
        {
            free(p->atoms);
            p->atoms = NULL;
        }
    p->num = 0;
}

void
deinit_var(struct variable *v)
{
    v->stack.pos = 0;
    deinit_proc(&v->proc);
}

void
deinit_var_ll(struct variable_ll *root)
{
    struct variable_ll *ll = root->next, *next = NULL;

    while (ll)
        {
            next = ll->next;
            free(ll);
            ll = next;
        }

    root->next = NULL;
}

void
deinit_vars(struct variable_ll *root)
{
    struct variable_ll *ll = root->next, *next = NULL;

    while (ll)
        {
            next = ll->next;
            deinit_var(&ll->v);
            free(ll);
            ll = next;
        }

    deinit_var(&root->v);
    root->next = NULL;
}

void
print_vars(struct variable_ll *root)
{
    struct variable_ll *ll = root;

    while (ll)
        {
            printf("[var %s]\n", ll->key);

            if(ll->v.proc.atoms)
                {
                    int i;
                    printf("\t[proc] ");
                    for(i = 0; i < ll->v.proc.num; ++i)
                        print_atom(&ll->v.proc.atoms[i]);
                    printf("\n");
                }

            printf("\t[stack] ");
            print_stack(&ll->v.stack);

            ll = ll->next;
        }
}

char *
get_var_name(struct variable_ll *root, struct variable *v)
{
    struct variable_ll *ll = root;

    while (ll)
        {
            if(v == &ll->v)
                return ll->key;

            ll = ll->next;
        }

    return "(NOT FOUND)";
}

struct variable*
find_var(struct variable_ll *root, char *key)
{
    struct variable_ll *ll = root;

    while (ll)
        {
            if(strcmp(key, ll->key) == 0)
                return &ll->v;

            ll = ll->next;
        }

    return NULL;
}

struct variable*
add_var(struct variable_ll *root, char *key)
{
    struct variable_ll *ll = root;

    while(ll->next)
        ll = ll->next;

    ll->next = calloc(1, sizeof(struct variable_ll));
    strcpy(ll->next->key, key);

    return &ll->next->v;
}

bool
remove_var(struct variable_ll *root, char *key)
{
    struct variable_ll *ll = root, *prev = NULL;

    while (ll)
        {
            if(strcmp(key, ll->key) == 0)
                {
                    prev->next = ll->next;
                    deinit_var(&ll->v);
                    free(ll);
                    return true;
                }

            prev = ll;
            ll = ll->next;
        }

    return false;
}

void
append_atom(struct variable *v, struct atom *a)
{
    v->proc.num++;
    v->proc.atoms = realloc(v->proc.atoms, v->proc.num * sizeof(struct atom));
    v->proc.atoms[v->proc.num - 1] = *a;
}

void
process_var(struct variable *v)
{
    int i;
    struct atom *a;
    struct value x;

    v->stack.pos = 0;

    for(i = 0; i < v->proc.num; ++i)
        {
            a = &v->proc.atoms[i];

            switch(a->type)
                {
                    case ATOM_FLOAT:
                        x.type = VALUE_FLOAT;
                        x.x.f = a->x.f;
                        push(&v->stack, x);
                        break;

                    case ATOM_STRING:
                        x.type = VALUE_STRING;
                        x.x.p = a->x.s;
                        push(&v->stack, x);
                        break;

                    case ATOM_FUNCTION:
                        a->x.fn(&v->stack);
                        break;

                    case ATOM_UGEN:
                        a->x.u.update(&v->stack, &a->x.u);
                        break;

                    case ATOM_VARIABLE:
                        if(a->x.v.v)
                            {
                                concat(&a->x.v.v->stack, &v->stack);
                            }
                        else
                            {
                                x.type = VALUE_ERROR;
                                push(&v->stack, x);
                            }
                        break;

                    case ATOM_ERROR:
                        x.type = VALUE_ERROR;
                        push(&v->stack, x);
                        break;
                }
        }
}

void
process_vars(struct variable_ll *root)
{
    struct variable_ll *ll = root->next;

    while(ll)
        {
            process_var(&ll->v);
            ll = ll->next;
        }

    process_var(&root->v);
}

void
replace_vars(struct variable_ll *root, struct variable_ll *new_root)
{
    struct variable_ll *ll = new_root;

    while(ll)
        {
            if(ll->v.proc.atoms)
                {
                    struct variable *var;

                    if((var = find_var(root, ll->key)))
                        deinit_var(var);
                    else
                        var = add_var(root, ll->key);

                    *var = ll->v;
                }
            else if(strcmp(ll->key, "out") && find_var(root, ll->key))
                {
                    printf("removing var: %s\n", ll->key);
                    remove_var(root, ll->key);
                }

            ll = ll->next;
        }

    deinit_var_ll(new_root);
    reset_var_pointers(root);
}

void
reset_var_pointers(struct variable_ll *root)
{
    struct variable_ll *ll = root;

    while(ll)
        {
            int i;
            for(i = 0; i < ll->v.proc.num; ++i)
                {
                    struct atom *a = &ll->v.proc.atoms[i];
                    if(a->type == ATOM_VARIABLE)
                        if((a->x.v.v = find_var(root, a->x.v.key)) == NULL)
                            printf("[ERROR] reference to removed variable '%s' in '%s'\n", a->x.v.key, ll->key);
                }

            ll = ll->next;
        }
}
