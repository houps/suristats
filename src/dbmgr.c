#include <stdio.h>
#include <sqlite3.h>

#include "dbmgr.h"
#include "counter.h"

#define SQL_CREATE_COUNTERS_TABLE  "CREATE TABLE T_COUNTER (C_NAME CHAR(32), C_THREAD VARCHAR(32), C_VALUE INTEGER, C_RUN INTEGER, C_UPTIME INTEGER)"

#define SQL_CREATE_RUNS_TABLE      "CREATE TABLE T_RUN (R_ID INTEGER, R_STARTDATE TIMESTAMP, R_ENDDATE TIMESTAMP, R_DURATION INTEGER, R_COMMENT VARCHAR(32))"

#define SQL_INSERT_COUNTER         "INSERT INTO T_COUNTER VALUES ('%31s', '%31s', %ld, %d, %d)"

#define SQL_INSERT_RUN             "INSERT INTO T_RUN VALUES ('%d', '%31s', %ld, %d)"

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int dbCreate(char * filename, struct counterList * list)
{
    sqlite3 *db;
    int ret;
    char *zErrMsg = 0;

    /* check that the db does not exist */
    //printf("%s, %16lx\n", filename, (long unsigned int)list);
    /* create it */
    ret = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if( ret ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    }
    /* 1 - create runs table */
    ret = sqlite3_exec(db, SQL_CREATE_COUNTERS_TABLE, callback, 0, &zErrMsg);
    if( ret!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    /* 2 - create counters table */
    ret = sqlite3_exec(db, SQL_CREATE_RUNS_TABLE, callback, 0, &zErrMsg);
    if( ret!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    /* fill the db if list provided */
    if (list != NULL) {
        int i, max;

        //printf("LIST %d\n", list->count);
        ret = sqlite3_exec(db, "BEGIN", callback, 0, &zErrMsg);
        if( ret!=SQLITE_OK ){
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
        //printf("ERROR %d\n", i);
        ret = sqlite3_exec(db, "COMMIT", callback, 0, &zErrMsg);
        if( ret!=SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
     }
    /* close the db file */
    sqlite3_close(db);
    return 0;
}

int dbDelete(char * filename)
{
    printf("%s", filename);
    return 0;
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
