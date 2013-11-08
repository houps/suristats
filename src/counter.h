#ifndef __COUNTER_H__
#   define __COUNTER_H__


    /* counter API */

    struct counter {
        struct counter *next;        /* pointer to the next element in the list */
        char           *name;        /* name of the counter */
        char           *thread_name; /* name of the thread */
        long long int   value;       /* value of the counter */
        unsigned int    run_id;      /* run identifier */
        unsigned int    uptime;      /* uptime */
    };

    struct counter * counterCreate(char *cname,
                                   char *tname,
                                   long long int value,
                                   unsigned int run_id,
                                   unsigned int uptime);
    void counterDelete(struct counter *c);
    void counterDisplay(struct counter *c);

    /* counterList API */

    struct counterList {
        struct counter *head;   /* pointer to the first element in the list */
        struct counter *tail;   /* pointer to the last element in the list */
        int             count;  /* number of elements in the list */
    };

    struct counterList * counterListCreate(void);
    void counterListDelete(struct counterList *l);
    void counterListAppend(struct counterList *l, struct counter *c);
    struct counter * counterListExtract(struct counterList *l);
    struct counter * counterListGetFirst(struct counterList *l);
    struct counter * counterListGetNext(struct counter *c);
    void counterListDisplay(struct counterList *l);

#endif
