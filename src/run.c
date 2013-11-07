#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#include "run.h"


/* TRACE MACRO */
#define TRACE(...)    //printf


struct run *runCreate(int id, char *start, unsigned int uptime)
{
    struct run *r;

    r = malloc(sizeof(struct run));
    if (r == NULL) {
        perror("runCreate - malloc failed\n");
        exit(EXIT_FAILURE);
    }
    r->next = NULL;
    r->id = id;
    r->startTime = strdup(start);
    r->uptime = uptime;
    TRACE("%s - id=%d date=%s uptime=%d\n", __FUNCTION__, r->id, r->start, r->uptime);
    runDisplay(r);
    return r;
}

void runUpdate(struct run *r, unsigned int uptime)
{
    assert(r != NULL);
    r->uptime = uptime;
    runDisplay(r);
}

int runDelete(struct run *r)
{
    assert(r->next == NULL);
    TRACE("%s - @=%16lx\n", __FUNCTION__, (long int)r);
    free(r->startTime);
    free(r);
    return 0;
}

void runDisplay(struct run * r)
{
    assert(r);
    printf("id=%d start=%s uptime=%d\n", r->id, r->startTime, r->uptime);
}

struct runList * runListCreate(void)
{
    struct runList * l = NULL;

    l = malloc(sizeof(struct runList));
    l->head = NULL;
    l->tail = NULL;
    l->count = 0;
    return l;
}

int runListDelete(struct runList *l)
{
    while (l->count != 0) {
        struct run * r = runListGetFirst(l);
        runDelete(r);
    }
    free(l);
    return 0;
}

int runListAppend(struct runList *l, struct run *r)
{
    int ret = -1;

    assert(l != NULL);
    assert(r != NULL);

    if (r->next == NULL) {
        l->count++;
        if (l->tail == NULL) {
            l->tail = r;
            l->head = r;
        } else {
            l->tail->next = r;
            l->tail = r;
        }
        ret = 0;
    }
    TRACE("%s: returned %d\n", __FUNCTION__, ret);
    return ret;
}

struct run *runListGetFirst(struct runList * l)
{
    struct run *r = NULL;

    if (l->count > 0) {
        r = l->head;
        l->head = r->next;
        r->next = NULL;
        l->count--;
        if (l->head == NULL) {
            l->tail = NULL;
        }
    }
    return r;
}

void runListDisplay(struct runList * l)
{
    int i;
    struct run *current;

    assert(l);
    current = l->head;
    for (i = 0; i < l->count; i++) {
        assert(current);
        runDisplay(current);
        current = current->next;
    }
    printf("%d run(s) in the list.\n", l->count);
    //assert(current == l->tail);
}

#ifdef TEST_UNIT
int main()
{
    struct runList *list;
    struct run *r;
    int i;

    printf("Unitary test of fichier library.\n");
    /* create the list */
    list = runListCreate();
    /* put runs in list */
    for (i = 0; i < 10; i++) {
        /* create counter */
        r = runCreate(i, "2013-11-05 20:12:30", 100);
        /* put it in the list */
        runListAppend(list, r);
    }
    /* display runs in list by traversing the list */
    runListDisplay(list);
    /* extract all runs from list */
    for (i = 0; i < 10; i++) {
        r = runListGetFirst(list);
        runListDisplay(list);
        runDelete(r);
    }
    return 0;
}
#endif
