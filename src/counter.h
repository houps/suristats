#ifndef __COUNTER_H__
#   define __COUNTER_H__


    /* counter API */

    struct counter {
        struct counter *next;
        char           *name;
        char           *thread_name;
        long int        value;
        unsigned int    run_id;
        unsigned int    uptime;
    };

    struct counter * counterCreate(char *cname, char *tname, long int value, unsigned int run_id, unsigned int uptime);

    int counterDelete(struct counter *c);

    void counterDisplay(struct counter * c);

    /* counterList API */

    struct counterList {
        struct counter *head;
        struct counter *tail;
        int             count;
    };

    struct counterList * counterListCreate(void);

    int counterListDelete(struct counterList *l);

    int counterListAppend(struct counterList *l, struct counter *c);

    struct counter * counterListGetFirst(struct counterList * l);
    
    void counterListDisplay(struct counterList * l);

#endif
