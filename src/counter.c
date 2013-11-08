#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#include "chi_debug.h"
#include "counter.h"


/*
 * 
 * name: counterCreate
 *     Allocates and initializes a counter.
 * @param cname
 *     counter's name
 * @param tname
 *     thread's name
 * @param value
 *     counter's value
 * @param run_id
 *     run identifier
 * @param uptime
 *     uptime for this counter's value
 * @return
 *     the pointer to the created counter
 * 
 */
struct counter * counterCreate(char *cname, char *tname, long long int value,
                               unsigned int run_id, unsigned int uptime)
{
    struct counter *c;

    c = malloc(sizeof(struct counter));
    if (c == NULL) {
        perror("counterCreate - malloc failed\n");
        exit(EXIT_FAILURE);
    }
    c->next = NULL;
    c->name = strdup(cname);
    c->thread_name = strdup(tname);
    c->value = value;
    c->run_id = run_id;
    c->uptime = uptime;
    debug_print("%s - cname=%s tname=%s value=%lld\n", __FUNCTION__, c->name, c->thread_name, c->value);
    return c;
}

/*
 * 
 * name: counterDelete
 *    Deletes a counter. Free memory allocated to it.
 * @param c
 *    pointer to the counter to be deleted
 * @return
 *    nothing
 */
void counterDelete(struct counter *c)
{
    assert(c->next == NULL);
    debug_print("%s - @=0x%llx\n", __FUNCTION__, (long long int)c);
    free(c->name);
    free(c->thread_name);
    free(c);
    return;
}

void counterDisplay(struct counter * c)
{
    assert(c);
    printf("name=%s thread=%s value=%lld run=%d up=%d\n", c->name, c->thread_name, c->value, c->run_id, c->uptime);
}

struct counterList * counterListCreate(void)
{
    struct counterList * l = NULL;

    l = malloc(sizeof(struct counterList));
    l->head = NULL;
    l->tail = NULL;
    l->count = 0;
    return l;
}

void counterListDelete(struct counterList *l)
{
    while (l->count != 0) {
        struct counter * c = counterListExtract(l);
        counterDelete(c);
    }
    free(l);
}

void counterListAppend(struct counterList *l, struct counter *f)
{
    assert(l != NULL);
    assert(f != NULL);
    assert(f->next == NULL);
    
    l->count++;
    if (l->tail == NULL) {
        l->tail = f;
        l->head = f;
    } else {
        l->tail->next = f;
        l->tail = f;
    }
    debug_print("%s - @=%16lx\n", __FUNCTION__, (long int)f);
}

struct counter * counterListExtract(struct counterList * l)
{
    struct counter *c = NULL;

    if (l->count > 0) {
        c = l->head;
        l->head = c->next;
        c->next = NULL;
        l->count--;
        if (l->head == NULL) {
            l->tail = NULL;
        }
    }
    return c;
}

struct counter * counterListGetFirst(struct counterList * l)
{
    return l->head;
}

struct counter * counterListGetNext(struct counter * c)
{
    return c->next;
}

void counterListDisplay(struct counterList * l)
{
    int i;
    struct counter * current;

    assert(l);
    current = l->head;
    for (i = 0; i < l->count; i++) {
        assert(current);
        counterDisplay(current);
        current = current->next;
    }
    printf("%d counter(s) in the list.\n", l->count);
}

/*
 * 
 * Standalone unitary tests
 * 
 */

#ifdef TEST_UNIT
int main()
{
    struct counterList *list;
    struct counter *c;
    int i;

    printf("Unitary test of fichier library.\n");
    /* create the list */
    list = counterListCreate();
    /* put counters in list */
    for (i = 0; i < 10; i++) {
        /* create counter */
        c = counterCreate("c0", "t0", i,0,0);
        /* put it in the list */
        counterListAppend(list, c);
    }
    /* display counters in list by traversing the list */
    counterListDisplay(list);
    /* extract all counters from list */
    for (i = 0; i < 10; i++) {
        c = counterListGetFirst(list);
        counterListDisplay(list);
        counterDelete(c);
    }

    return 0;
}
#endif
