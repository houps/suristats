#ifndef __DBMGR_H__

#   include <sqlite3.h>

#   include "counter.h"
#   include "run.h"


    int       dbCreate(char * filename, struct counterList * clist, struct runList * rlist);

    int       dbRead(char * filename, struct counterList * clist, struct runList * rlist);

    sqlite3 * dbOpen(char * filename);
    
    void      dbRunInsert(sqlite3 * db, int run, char * startDate, int uptime);

    void      dbRunUpdate(sqlite3 * db, int run, int uptime);

    void      dbClose(sqlite3 * db);

#endif
