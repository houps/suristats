#include <stdio.h>
#include <stdlib.h> /* exit() */
#include <unistd.h> /* access() */
#include <sqlite3.h>
#include <assert.h>

#include "dbmgr.h"
#include "counter.h"


#define UNUSED_PARAMETER(_param_)   (void)(_param_)

#define SQL_CREATE_TABLE_COUNTERS  "CREATE TABLE T_COUNTER (C_NAME CHAR(32), C_THREAD VARCHAR(32), C_VALUE INTEGER, C_RUN INTEGER, C_UPTIME INTEGER)"

#define SQL_CREATE_TABLE_RUNS      "CREATE TABLE T_RUN (R_ID INTEGER, R_STARTDATE DATETIME, R_ENDDATE DATETIME, R_DURATION INTEGER, R_COMMENT VARCHAR(32))"

#define SQL_INSERT_COUNTER "INSERT INTO T_COUNTER VALUES ('%31s', '%31s', %ld, %d, %d)"

#define SQL_INSERT_RUN     "INSERT INTO T_RUN VALUES ('%d', datetime('%s', '-%d seconds'), datetime('%s'), %d, '%s')"

#define SQL_UPDATE_RUN  "UPDATE T_RUN SET R_ENDDATE = datetime(R_ENDDATE, '+%d seconds'), R_UPTIME = %d WHERE R_ID = %d"


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

int dbGetRunNumber(sqlite3 *db)
{
    char *zErrMsg = 0;
    int ret;
    int number;
    
    /* get run numbers from db */

#define SQL_RUN_LIST "SELECT COUNT(*) FROM T_RUN"

    ret = sqlite3_exec(db, SQL_RUN_LIST, callback_getCount, &number, &zErrMsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    return number;
}

int dbRead(char * filename, struct counterList * clist, struct runList * rlist)
{
    sqlite3 *db;
    int ret, i, runs;
    char *zErrMsg = 0;
    //struct runList * rlist;
    struct run * r;

    assert(clist != NULL);
    assert(rlist != NULL);
    
    /* open it */
    ret = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READONLY, NULL);
    if (ret) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    }
    /* extract runs info from the db */
#define SQL_RUN_LIST "SELECT * FROM T_RUN"

    //rlist = runListCreate();
    ret = sqlite3_exec(db, SQL_RUN_LIST, callback_runs, rlist, &zErrMsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    //runListDelete(rlist);
	runs = rlist->count;
 
    /* extract counters info from the db */
    for (i = 0, r = runListGetFirst(rlist); i < rlist->count; i++, r = runListGetNext(r)) {
        char request[256];
        int number;
        
#define SQL_SELECT_COUNT_PER_RUN "SELECT COUNT(*) FROM T_COUNTER WHERE C_RUN = %d"

        snprintf(request, 256, SQL_SELECT_COUNT_PER_RUN, i);
        ret = sqlite3_exec(db, request, callback_getCount, &number, &zErrMsg);
        if (ret != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        printf("%d | start: %s | uptime: %d seconds | %d counters.\n",
               r->id, r->startTime, r->uptime, number);
    }
    /* close the db file */
    sqlite3_close(db);
    return 0;
}

int dbStatPrint(char * filename)
{
    sqlite3 *db;
    int ret, i, runs;
    char *zErrMsg = 0;
    struct run * r;
    struct runList * rlist;
    struct counterList * clist;
    
    assert(clist != NULL);
    
    /* open db */
    ret = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READONLY, NULL);
    if (ret) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    }
    
    /* get run numbers from db */
	runs = dbGetRunNumber(db);
    printf("Number of run(s): %d\n", runs);
#if 0
    /* get threads list from db */

#define SQL_THREAD_LIST "SELECT C_THREAD FROM T_COUNTER, T_RUN WHERE (C_RUN = %d, C_UPTIME = R_UPTIME, C_NAME = capture.kernel_packets)"

    ret = sqlite3_exec(db, SQL_THREAD_LIST, callback_runs, rlist, &zErrMsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    printf("Number of run(s): %d\n", rlist->count);

    /* extract required counters from the db for each run */
    for (i = 0, r = runListGetFirst(rlist); i < rlist->count; i++, r = runListGetNext(r)) {
        char request[256];
        
#define SQL_STATS_LIST_COUNTERS  "SELECT * FROM T_COUNTER WHERE (C_RUN = %d, C_THREAD = %s%d, C_NAME = %s)"

        snprintf(request, 256, SQL_STATS_LIST_COUNTERS, i, "AFPacketeth3", i, "capture.kernel_packets");
        ret = sqlite3_exec(db, request, callback_counters_list, clist, &zErrMsg);
        if (ret != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        printf("run: %d | start: %s | uptime: %d seconds | %d counters.\n",
               r->id, r->startTime, r->uptime, number);
    }

//#define SQL_UPDATE_RUN  "UPDATE T_RUN SET R_ENDDATE = datetime(R_ENDDATE, '+%d seconds'), R_UPTIME = %d WHERE R_ID = %d"
#define SQL_STATS_LIST_COUNTERS  "SELECT * FROM T_COUNTER WHERE (C_RUN = %d, C_THREAD = %s, C_NAME = %s)"
//#define SQL_STATS_LIST_DROPS    "SELECT * FROM T_COUNTER"
    snprintf(request, 256, SQL_SELECT_COUNT_PER_RUN, i);
    ret = sqlite3_exec(db, SQL_RUN_LIST, callback_counters, clist, &zErrMsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    printf("Number of run(s): %d\n", rlist->count);
    /* extract counters info from the db */
     for (i = 0, r = runListGetFirst(rlist); i < rlist->count; i++, r = runListGetNext(r)) {
        char request[256];
        int number;
        
#define SQL_SELECT_COUNT_PER_RUN "SELECT COUNT(*) FROM T_COUNTER WHERE C_RUN = %d"

        snprintf(request, 256, SQL_SELECT_COUNT_PER_RUN, i);
        ret = sqlite3_exec(db, request, callback_counters, &number, &zErrMsg);
        if (ret != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        printf("run: %d | start: %s | uptime: %d seconds | %d counters.\n",
               r->id, r->startTime, r->uptime, number);
    }
#endif
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
