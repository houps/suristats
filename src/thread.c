#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

//#define DEBUG

#include "chi_debug.h"
#include "thread.h"


struct thread *threadCreate(char *name)
{
    struct thread *t;

    t = malloc(sizeof(struct thread));
    if (t == NULL) {
        perror("threadCreate - malloc failed\n");
        exit(EXIT_FAILURE);
    }
    t->next = NULL;
    t->name = strdup(name);
    t->packets = 0;
    t->drops = 0;
    debug_print("%s - name=%s\n", __FUNCTION__, t->name);
    return t;
}

void threadDelete(struct thread *t)
{
    assert(t->next == NULL);
    debug_print("%s - @=%16lx\n", __FUNCTION__, (long int)t);
    free(t->name);
    free(t);
    return;
}

void threadDisplay(struct thread * t)
{
    assert(t);
    printf("name=%s\n", t->name);
}

struct threadList * threadListCreate(void)
{
    struct threadList * l = NULL;

    l = malloc(sizeof(struct threadList));
    l->head = NULL;
    l->tail = NULL;
    l->count = 0;
    return l;
}

void threadListDelete(struct threadList *l)
{
    while (l->count != 0) {
        struct thread * t = threadListExtract(l);
        threadDelete(t);
    }
    free(l);
}

void threadListAppend(struct threadList *l, struct thread *t)
{
    assert(l != NULL);
    assert(t != NULL);
    assert(t->next == NULL);
        
    l->count++;
    if (l->tail == NULL) {
        l->tail = t;
        l->head = t;
    } else {
        l->tail->next = t;
        l->tail = t;
    }
    debug_print("%s - @=%16lx\n", __FUNCTION__, (long int)t);
}

struct thread *threadListExtract(struct threadList * l)
{
    struct thread *t = NULL;

    if (l->count > 0) {
        t = l->head;
        l->head = t->next;
        t->next = NULL;
        l->count--;
        if (l->head == NULL) {
            l->tail = NULL;
        }
    }
    return t;
} /* threadListPopFirst() */

/*
 * 
 * name: threadListTravel
 * @param l is the list to be traversed.
 * @param callback is function that will be called for each element in the list.
 *        The callback return value is either '1' if the traversal of the list
 *        must be interrupted or '0' if we must continue. 
 * @param param is the first parameter passed to the callback.
 * @return '0' if OK or '1' if the traversal has been interrupted.
 * 
 */
int threadListTravel(struct threadList * l,
                     int (*callback)(void*, struct thread*),
                     void * param)
{
    int ret;
    struct thread * t;
    
    t = threadListGetFirst(l);
    do {
        ret = callback(param, t);
        t = threadListGetNext(t);
    } while (t != NULL);
    return ret;
} /* threadListTravel */

struct thread *threadListGetFirst(struct threadList * l)
{
    return l->head;
}

struct thread *threadListGetNext(struct thread * r)
{
    return r->next;
}

void threadListDisplay(struct threadList * l)
{
    int i;
    struct thread *current;

    assert(l);
    current = l->head;
    for (i = 0; i < l->count; i++) {
        assert(current);
        threadDisplay(current);
        current = current->next;
    }
    printf("%d thread(s) in the list.\n", l->count);
}

/*
 * 
 * Standalone unitary tests
 * 
 */

#ifdef TEST_UNIT
int main()
{
    struct threadList *list;
    struct thread *t;
    int i;

    printf("Unitary test of fichier library.\n");
    /* create the list */
    list = threadListCreate();
    /* put threads in list */
    for (i = 0; i < 10; i++) {
        char temp[32];
        
        snprintf(temp, 31, "nom_%d", i);
        /* create counter */
        t = threadCreate(temp);
        /* put it in the list */
        threadListAppend(list, t);
    }
    /* display threads in list by traversing the list */
    threadListDisplay(list);
    /* extract all threads from list */
    for (i = 0; i < 10; i++) {
        t = threadListGetFirst(list);
        threadListDisplay(list);
        threadDelete(t);
    }
    return 0;
}
#endif
