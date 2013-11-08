#ifndef __THREAD_H__
#   define __THREAD_H__

    /* thread API */

    struct thread {
        struct thread *next;
        char          *name;
        long long int  packets;
        long long int  drops;
    };

    struct thread *threadCreate(char *name);
    void threadDelete(struct thread *t);
    void threadDisplay(struct thread *t);

    /* threadList API */

    struct threadList {
        struct thread *head;   /* pointer to the first element in the list */
        struct thread *tail;   /* pointer to the last element in the list */
        int            count;  /* number of elements in the list */
    };

    struct threadList *threadListCreate(void);
    void threadListDelete(struct threadList *l);
    void threadListAppend(struct threadList *l, struct thread *t);
    struct thread *threadListExtract(struct threadList *l);
    struct thread *threadListGetFirst(struct threadList *l);
    struct thread *threadListGetNext(struct thread *t);
    void threadListDisplay(struct threadList *l);
    int threadListTravel(struct threadList *l,
                         int (*callback)(void *param, struct thread * t),
                         void *param);

#endif
