#include "stak.h"

variable_ll_t variables[2];

/* variable */
void
variable_print(variable_t *variable)
{
    printf("[variable] \"%s\" %p\n", variable->key, (void*)variable);

    procedure_print(&variable->procedure);
    stack_print(&variable->stack);
}

void
variable_deinit(variable_t *variable)
{
    stack_reset(&variable->stack);
    procedure_deinit(&variable->procedure);
}

void
variable_process(variable_t *variable)
{
    procedure_process(&variable->procedure, &variable->stack);
}

void
variable_reset_process_type(variable_t *variable)
{
    procedure_reset_process_type(&variable->procedure);
}

void
variable_reset_pointers(variable_ll_t *root, variable_t *variable)
{
    int i;

    if(variable->procedure.atoms == NULL)
        return;

    for(i = 0; i < variable->procedure.len; ++i)
        {
            atom_t *atom = &variable->procedure.atoms[i];

            if(atom->type == ATOM_VARIABLE)
                atom->x.variable_key.variable = variable_ll_find(root, atom->x.variable_key.key);
        }
}
/* */



/* variable_ll */
void
variable_ll_print(variable_ll_t *root)
{
    variable_ll_t *ll = root;

    printf("[variable_ll] %p\n", (void*)root);

    while(ll != NULL)
        {
            if(ll->variable != NULL)
                variable_print(ll->variable);

            ll = ll->next;
        }

    printf("\n");
}

void
variable_ll_init(variable_ll_t *root)
{
    memset(root, 0, sizeof(variable_ll_t));
}

void
variable_ll_free(variable_ll_t *root)
{
    variable_ll_t *ll = root, *next = NULL;

    while(ll != NULL)
        {
            next = ll->next;

            if(ll == root)
                ll->next = NULL;
            else
                free(ll);

            ll = next;
        }
}

void
variable_ll_deinit(variable_ll_t *root)
{
    variable_ll_t *ll = root, *next = NULL;

    while(ll != NULL)
        {

            next = ll->next;

            if(ll->variable != NULL)
                {
                    variable_deinit(ll->variable);
                    free(ll->variable);
                    ll->variable = NULL;
                }

            if(ll == root)
                ll->next = NULL;
            else
                free(ll);

            ll = next;
        }
}

variable_ll_t *
variable_ll_add(variable_ll_t *root, char *key)
{
    variable_ll_t *ll = root;

    if(root->variable != NULL)
        {
            while(ll->next != NULL)
                ll = ll->next;

            ll->next = calloc(1, sizeof(variable_ll_t));
            ll = ll->next;
        }

    ll->variable = calloc(1, sizeof(variable_t));
    strncpy(ll->variable->key, key, KEY_LENGTH);

    return ll;
}

void
variable_ll_remove(variable_ll_t *root, char *key)
{
    variable_ll_t *ll = root, *prev = NULL;

    while(ll != NULL)
        {
            if(strncmp(key, ll->variable->key, KEY_LENGTH) == 0)
                {
                    if(ll->variable != NULL)
                        {
                            variable_deinit(ll->variable);
                            free(ll->variable);
                            ll->variable = NULL;
                        }

                    if(prev != NULL)
                        prev->next = ll->next;

                    if(ll != root)
                        free(ll);

                    return;
                }

            prev = ll;
            ll = ll->next;
        }
}

variable_t *
variable_ll_find(variable_ll_t *root, char *key)
{
    variable_ll_t *ll = variable_ll_find_ll(root, key);

    return (ll == NULL) ? NULL : ll->variable;
}

variable_ll_t *
variable_ll_find_ll(variable_ll_t *root, char *key)
{
    variable_ll_t *ll = root;

    while(ll != NULL)
        {
            if(ll->variable != NULL)
                if(strcmp(key, ll->variable->key) == 0)
                    return ll;

            ll = ll->next;
        }

    return NULL;
}

void
variable_ll_merge(variable_ll_t *old, variable_ll_t *new)
{
    variable_ll_t *ll = new;

    while(ll != NULL)
        {
            if(ll->variable != NULL)
                {
                    if(ll->variable->procedure.atoms == NULL)
                        {
                            variable_ll_remove(old, ll->variable->key);

                            variable_deinit(ll->variable);
                            free(ll->variable);
                        }
                    else
                        {
                            variable_ll_t *found = variable_ll_find_ll(old, ll->variable->key);

                            if(found == NULL)
                                found = variable_ll_add(old, ll->variable->key);

                            if(found != NULL)
                                {
                                    if(found->variable != NULL)
                                        {
                                            variable_deinit(found->variable);
                                            free(found->variable);
                                        }

                                    found->variable = ll->variable;
                                }
                        }
                }

            ll = ll->next;
        }

    variable_ll_free(new);
    variable_ll_refresh(old);
}

void
variable_ll_process(variable_ll_t *root)
{
    variable_ll_t *ll = root;

    while(ll != NULL)
        {
            if(ll->variable != NULL)
                variable_process(ll->variable);

            ll = ll->next;
        }
}

void
variable_ll_refresh(variable_ll_t *root)
{
    variable_ll_t *ll = root;

    while(ll != NULL)
        {
            if(ll->variable != NULL)
                {
                    variable_reset_pointers(root, ll->variable);
                    variable_reset_process_type(ll->variable);
                }

            ll = ll->next;
        }
}
/* */
