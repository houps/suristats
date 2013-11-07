#include <stdio.h>
#include <sqlite3.h>

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

int dbCreate(char * filename, struct counterList * clist, struct runList * rlist)
{
    sqlite3 *db;
    int ret;
    char *zErrMsg = 0;

    /* check that the db does not exist */
    //printf("%s, %16lx\n", filename, (long unsigned int)list);
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
    /* fill the db if list provided */
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

            c = counterListGetFirst(clist);
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
    /* fill the db if list provided */
    if (rlist != NULL) {
        int i, max;

        max = rlist->count;
        for (i = 0; i < max; i++) {
            char request[256];
            struct run * r;

            r = runListGetFirst(rlist);
            snprintf(request, 256, SQL_INSERT_RUN, r->id, r->startTime, r->uptime, r->startTime, r->uptime, "");
//#define SQL_INSERT_RUN     "INSERT INTO T_RUN VALUES ('%d', datetime('%s', '-%d seconds'), datetime('%s'), %d, '%s')"
            printf("%s\n", request);
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

int dbCreate2(char * filename, struct counterList * list)
{
    sqlite3 *db;
    int ret;
    char *zErrMsg = 0;

    /* check that the db does not exist */
    //printf("%s, %16lx\n", filename, (long unsigned int)list);
    /* create it */
    ret = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if(ret){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    }
    /* 1 - create runs table */
    ret = sqlite3_exec(db, SQL_CREATE_TABLE_COUNTERS, callback, 0, &zErrMsg);
    if( ret!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    /* 2 - create counters table */
    ret = sqlite3_exec(db, SQL_CREATE_TABLE_RUNS, callback, 0, &zErrMsg);
    if( ret!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    /* fill the db if list provided */
    if (list != NULL) {
        int i, max;

        ret = sqlite3_exec(db, "BEGIN", callback, 0, &zErrMsg);
        if (ret != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }

        max = list->count;
        for (i = 0; i < max; i++) {
            char request[256];
            struct counter * c;

            c = counterListGetFirst(list);
            snprintf(request, 256, SQL_INSERT_COUNTER, c->name, c->thread_name, c->value, c->run_id, c->uptime);
            counterDelete(c);
            ret = sqlite3_exec(db, request, callback, 0, &zErrMsg);
            if( ret!=SQLITE_OK ){
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
