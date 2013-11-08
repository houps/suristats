#ifndef __RUN_H__
#   define __RUN_H__


    /* run API */

    struct run {
        struct run   *next;        /* pointer to the next element in the list */
        int           id;          /* identifier of the run */
        char         *startTime;   /* date and time (as a string) of the first *
                                    * dump for this run (YYYY-MM-DD hh:mm:ss)  */
        unsigned int  firstUptime; /* uptime of the first dump for this run,         *
                                    * used to calculate the real start date and time */
        unsigned int  uptime;      /* duration of the run */
    };

    struct run *runCreate(int id, char *start, unsigned int uptime);
    void runUpdate(struct run *r, unsigned int uptime);
    void runDelete(struct run *r);
    void runDisplay(struct run *r);

    /* runList API */

    struct runList {
        struct run *head;
        struct run *tail;
        int         count;
    };

    struct runList *runListCreate(void);
    void runListDelete(struct runList *l);
    void runListAppend(struct runList *l, struct run *c);
    struct run *runListExtract(struct runList *l);
    struct run *runListGetFirst(struct runList *l);
    struct run *runListGetNext(struct run * r);
    void runListDisplay(struct runList *l);

#endif
