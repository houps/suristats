#ifndef __DBMGR_H__

#   include <sqlite3.h>

#   include "counter.h"
#   include "run.h"


    int dbCreate(char * filename);           /* name of the DB file */ 
    int dbFill(char * filename,              /* name of the DB file */
               struct counterList * clist,   /* list of counters to insert in DB */
               struct runList * rlist);      /* list of runs to insert in DB */
    int dbStatPrint(char * filename);        /* name of the DB file */

#endif
