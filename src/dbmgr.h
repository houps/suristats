#ifndef __DBMGR_H__

#   include <stdio.h>
#   include <sqlite3.h>

#   include "counter.h"

    int dbCreate(char * filename, struct counterList * list);

    int dbDelete(char * filename);

    int dbCreate2(char * filename, struct counterList * list);

    int dbCreate3(char * filename, struct counterList * list);

#endif
