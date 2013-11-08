#include <stdio.h>
#include <stdlib.h> /* exit() */
#include <unistd.h> /* access() */
#include <string.h> /* strcmp() */
#include <sqlite3.h>
#include <assert.h>

#include "dbmgr.h"
#include "counter.h"
#include "thread.h"

#define DEBUG        0

/*****************/
/* Useful Macros */
/*****************/

#define UNUSED_PARAMETER(_param_)   (void)(_param_)

#ifdef DEBUG
#   define DEBUG_TEST 1
#else
#   define DEBUG_TEST 0
#endif

#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

/****************/
/* SQL requests */
/****************/

#define SQL_CREATE_TABLE_COUNTERS  "CREATE TABLE T_COUNTER (C_NAME CHAR(32), C_THREAD VARCHAR(32), C_VALUE INTEGER, C_RUN INTEGER, C_UPTIME INTEGER)"

#define SQL_CREATE_TABLE_RUNS      "CREATE TABLE T_RUN (R_ID INTEGER, R_STARTDATE DATETIME, R_ENDDATE DATETIME, R_DURATION INTEGER, R_COMMENT VARCHAR(32))"

#define SQL_INSERT_COUNTER "INSERT INTO T_COUNTER VALUES ('%s', '%s', %ld, %d, %d)"

#define SQL_INSERT_RUN     "INSERT INTO T_RUN VALUES ('%d', datetime('%s', '-%d seconds'), datetime('%s'), %d, '%s')"

#define SQL_UPDATE_RUN  "UPDATE T_RUN SET R_ENDDATE = datetime(R_ENDDATE, '+%d seconds'), R_UPTIME = %d WHERE R_ID = %d"

#define SQL_STATS_LIST_COUNTERS  "SELECT * FROM T_COUNTER WHERE C_RUN = %d AND C_NAME = '%s'"

#define SQL_STATS_LIST_THREADS   "SELECT DISTINCT C_THREAD FROM T_COUNTER WHERE C_RUN = '%d' AND C_NAME='capture.kernel_packets'"


static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    int i;
    
    UNUSED_PARAMETER(NotUsed);
    for (i = 0; i < argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

static int callback_runs(void *list, int argc, char **argv, char **azColName) {
    struct runList * rlist = (struct runList *)list;
    struct run * run;
    
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(azColName);
    run = runCreate(atoi(argv[0]), argv[1], atoi(argv[3]));
    runListAppend(rlist, run);
    return 0;
}

static int callback_getCount(void *param, int argc, char **argv, char **azColName) {
    int * value_p = (int *)param;
    
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(azColName);
    *value_p = atoi(argv[0]);
    return 0;
}

static int callback_counters_list(void *param, int argc, char **argv, char **azColName) {
    struct counterList * clist = (struct counterList *)param;
	struct counter * c;
	
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(azColName);
    c = counterCreate(argv[0], argv[1], atol(argv[2]), atoi(argv[3]), atoi(argv[4]));
    counterListAppend(clist, c);
    return 0;
}

static int callback_threads_list(void *param, int argc, char **argv, char **azColName) {
    struct threadList * tlist = (struct threadList *)param;
	struct thread * t;
	
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(azColName);
    t = threadCreate(argv[0]);
    threadListAppend(tlist, t);
    return 0;
}

int dbCreate(char * filename, struct counterList * clist, struct runList * rlist)
{
    sqlite3 *db;
    int ret;
    char *zErrMsg = 0;

    /* check that the db does not exist */
    if (!access(filename, R_OK)) {
        perror("File already exists\n");
        exit(EXIT_FAILURE);
    }
    /* create it */
    ret = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    }
    /* 1 - create runs table */
    ret = sqlite3_exec(db, SQL_CREATE_TABLE_COUNTERS, callback, 0, &zErrMsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    /* 2 - create counters table */
    ret = sqlite3_exec(db, SQL_CREATE_TABLE_RUNS, callback, 0, &zErrMsg);
    if (ret != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    /* fill the db if counters' list provided */
    if (clist != NULL) {
        int i, max;

        ret = sqlite3_exec(db, "BEGIN", callback, 0, &zErrMsg);
        if (ret != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }

        max = clist->count;
        for (i = 0; i < max; i++) {
            char request[256];
            struct counter * c;

            c = counterListPopFirst(clist);
            snprintf(request, 256, SQL_INSERT_COUNTER, c->name, c->thread_name, c->value, c->run_id, c->uptime);
            counterDelete(c);
            ret = sqlite3_exec(db, request, callback, 0, &zErrMsg);
            if (ret != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
            }
        }
        ret = sqlite3_exec(db, "COMMIT", callback, 0, &zErrMsg);
        if (ret != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
    }
    /* fill the db if runs' list provided */
    if (rlist != NULL) {
        int i, max;

        max = rlist->count;
        for (i = 0; i < max; i++) {
            char request[256];
            struct run * r;

            r = runListPopFirst(rlist);
            snprintf(request, 256, SQL_INSERT_RUN, r->id, r->startTime, r->uptime, r->startTime, r->uptime, "");
            runDelete(r);
            ret = sqlite3_exec(db, request, callback, 0, &zErrMsg);
            if (ret != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
            }
        }
    }
    /* close the db file */
    sqlite3_close(db);
    return 0;
}

struct runList * dbGetRunList(sqlite3 *db)
{
    char *zErrMsg = 0;
    int ret;
    struct runList * rlist;
    
#define SQL_RUN_LIST  "SELECT * FROM T_RUN"

    rlist = runListCreate();
    ret = sqlite3_exec(db, SQL_RUN_LIST, callback_runs, rlist, &zErrMsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    return rlist;
}

int dbRead(char * filename)
{
    sqlite3 *db;
    int ret, run_id;
    char *zErrMsg = 0;
    struct runList * rlist;
    struct run * r;
    
    /* open it */
    ret = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READONLY, NULL);
    if (ret) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    }

    /* extract runs info from the db */
    rlist = runListCreate();
    ret = sqlite3_exec(db, SQL_RUN_LIST, callback_runs, rlist, &zErrMsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    printf("Number of run(s): %d\n", rlist->count);

    /* extract counters info from the db */
    for (run_id = 0, r = runListGetFirst(rlist); run_id < rlist->count; run_id++, r = runListGetNext(r)) {
        char request[256];
        int number;
        
#define SQL_SELECT_COUNT_PER_RUN "SELECT COUNT(*) FROM T_COUNTER WHERE C_RUN = %d"

        snprintf(request, 256, SQL_SELECT_COUNT_PER_RUN, run_id);
        ret = sqlite3_exec(db, request, callback_getCount, &number, &zErrMsg);
        if (ret != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        printf("%d | start: %s | uptime: %d seconds | %d counters.\n",
               r->id, r->startTime, r->uptime, number);
    }
    runListDelete(rlist);
    /* close the db file */
    sqlite3_close(db);
    return 0;
}

int dbStatPrint(char * filename)
{
    sqlite3 *db;
    int ret, run_id;
    char *zErrMsg = 0;
    struct runList * runsList;
    struct run * r;
    
    /* open db */
    ret = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READONLY, NULL);
    if (ret) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    }
    
    /* get run numbers from db */
	runsList = dbGetRunList(db);
    printf("Number of run(s): %d\n", runsList->count);

    /* for each run extract packets and drops lists to compute stats */
    for (run_id = 0, r = runListGetFirst(runsList); run_id < runsList->count; run_id++, r = runListGetNext(r)) {
        char request[256];
        struct counterList * packetList;
        struct counterList * dropList;
        struct threadList  * threadsList;
        struct counter * c;
        struct thread * t;
        int i;
        
        /* get the list of threads that contains "capture.kernel_packets" counter */
        threadsList = threadListCreate();
        snprintf(request, 256, SQL_STATS_LIST_THREADS, run_id);
        ret = sqlite3_exec(db, request, callback_threads_list, threadsList, &zErrMsg);
        if (ret != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        
        /* get the list of ALL capture.kernel_packets counters */
        packetList = counterListCreate();
        snprintf(request, 256, SQL_STATS_LIST_COUNTERS, run_id, "capture.kernel_packets");
        ret = sqlite3_exec(db, request, callback_counters_list, packetList, &zErrMsg);
        if (ret != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        
        /* get the list of ALL capture.kernel_drops counters */
        dropList = counterListCreate();
        snprintf(request, 256, SQL_STATS_LIST_COUNTERS, run_id, "capture.kernel_drops");
        ret = sqlite3_exec(db, request, callback_counters_list, dropList, &zErrMsg);
        if (ret != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }

        /* packetList traversal to get per thread stats */
        long long int packetsSum = 0;
        for (i = 0, c = counterListGetFirst(packetList); i < packetList->count; i++, c = counterListGetNext(c)) {
            debug_print("%d / %d : %lld -- ", i, packetList->count, packetsSum);
            packetsSum += c->value;
            /* traverse threadsList to increase its packet number */
            t = threadListGetFirst(threadsList);
            while (strcmp(c->thread_name, t->name)) {
                t = threadListGetNext(t);
            }
            t->packets += c->value;
            debug_print("%s %lld\n", t->name, t->packets);
        }
        debug_print("uptime=%d\n", r->uptime);
        debug_print("%d - %lld\n", packetList->count, packetsSum);

        /* dropsList traversal to get per thread stats */
        long long int dropsSum = 0;
        for (i = 0, c = counterListGetFirst(dropList); i < dropList->count; i++, c = counterListGetNext(c)) {
            dropsSum += c->value;
            /* traverse threadsList to increase its drop number */
            t = threadListGetFirst(threadsList);
            while (strcmp(c->thread_name, t->name)) {
                t = threadListGetNext(t);
            }
            t->drops += c->value;
        }
        debug_print("%d - %lld\n", dropList->count, dropsSum);

        /* Calculate the ratio drops/packets */
        double ratio = (double)dropsSum/(double)packetsSum;
        printf("\n-------------------------------------------------------------\n");
        printf("run %d\ndrops/packets ratio= %f\n\n", run_id, ratio);
        printf("-------------------------------------------------------------\n");
        /* Print statistics per thread */
        printf("%-32s|  packets/s   |  drops/s\n","Thread name");
        printf("-------------------------------------------------------------\n");
        for (i = 0, t = threadListGetFirst(threadsList); i < threadsList->count; i++, t = threadListGetNext(t)) {
            printf("%-32s| %12.2f | %12.2f\n",t->name,
                   (double)t->packets/(double)r->uptime,
                   (double)t->drops/(double)r->uptime);
        }
        
        /* Free memory */
        threadListDelete(threadsList);
        counterListDelete(packetList);
        counterListDelete(dropList);
    }
    runListDelete(runsList);
    /* close the db file */
    sqlite3_close(db);
    return 0;
	
}


void dbRunInsert(sqlite3 *db, int run, char * startDate, int uptime)
{
    char request[256];
    int ret;
    char *zErrMsg = 0;
    
    snprintf(request, 256, SQL_INSERT_RUN, run, startDate, uptime, startDate, uptime, "");
    ret = sqlite3_exec(db, request, callback, 0, &zErrMsg);
    if(ret != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

void dbRunUpdate(sqlite3 *db, int run, int uptime)
{
    char request[256];
    int ret;
    char *zErrMsg = 0;
   
    snprintf(request, 256, SQL_UPDATE_RUN, uptime, uptime, run);
    ret = sqlite3_exec(db, request, callback, 0, &zErrMsg);
    if(ret != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

#ifdef TEST_UNIT
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int main(int argc, char **argv){
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    if( argc!=3 ){
        fprintf(stderr, "Usage: %s DATABASE SQL-STATEMENT\n", argv[0]);
        return(1);
    }
    rc = sqlite3_open(argv[1], &db);
    if( rc ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    }
    rc = sqlite3_exec(db, argv[2], callback, 0, &zErrMsg);
    if( rc!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
    return 0;
}
#endif
