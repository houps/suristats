#ifndef __RUN_H__
#   define __RUN_H__


    /* run API */

    struct run {
        struct run   *next;
        int           id;
        char          startTime[32];
        unsigned int  uptime;
    };

    struct run *runCreate(int id, char *start, unsigned int uptime);

    void runUpdate(struct run *r, unsigned int uptime);

    int runDelete(struct run *r);

    void runDisplay(struct run *r);

    /* runList API */

    struct runList {
        struct run *head;
        struct run *tail;
        int         count;
    };

    struct runList *runListCreate(void);

    int runListDelete(struct runList *l);

    int runListAppend(struct runList *l, struct run *c);

    struct run *runListGetFirst(struct run *l);
    
    void runListDisplay(struct runList *l);

#endif
