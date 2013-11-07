#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  /* getopt() */
#include <string.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <limits.h> /* INT_MAX */

#include "counter.h"
#include "run.h"
#include "dbmgr.h"

/* TRACE MACRO */
#define TRACE() //printf()

int ParseLogFile(char * filename, struct counterList * clist, struct runList * rlist)
{
    FILE * fd;
    int i, ret;
    char line[128];
    int run_id = -1;
    int uptime = 0;
    int prev_uptime = INT_MAX;
    struct run * run;

    /* open file */
    fd = fopen(filename, "r");
    if (fd == NULL) {
        perror("File opening error.");
        exit(EXIT_FAILURE);
    }
    while(fgets(line, 128, fd) != NULL) {

        char counter_name[80];
        char TM_name[32]; 
        long int  counter_value;
        
        ret = sscanf(line, "%31s | %31s | %ld\n", counter_name, TM_name, &counter_value);
        if (ret == 3) {
            struct counter * cnt = counterCreate(counter_name, TM_name, counter_value, run_id, uptime);
            counterListAppend(clist, cnt);
        }
        else {
            /* read header (4 more lines) */
            for (i = 0; i < 4; i++) {
                char *c;
                int month, day, year, uptime_days, uptime_hours, uptime_minutes, uptime_seconds;
                char time[32];

                c = fgets(line, 128, fd);
                if (c == NULL) {
                    perror("Corrupted file.");
                    exit(EXIT_FAILURE);
                }
                ret = sscanf(line, "Date: %d/%d/%d -- %s (uptime: %dd, %02dh %02dm %02ds)\n",
                             &month, &day, &year, time,
                             &uptime_days, &uptime_hours, &uptime_minutes, &uptime_seconds);
                if (ret == 8) {
                    uptime = uptime_days * 24 * 3600 + uptime_hours * 3600 + uptime_minutes * 60 + uptime_seconds;
                    if (uptime <= prev_uptime) {
                        char start[32];
                        
                        /* new run */
                        run_id++;
                        snprintf(start, 31, "%04d-%02d-%02d %s", year, month, day, time);
                        /* create run and append it to the run list*/
                        run = runCreate(run_id, start, uptime);
                        runListAppend(rlist, run);
                        prev_uptime = uptime;
                    }
                    else {
                        runUpdate(run, uptime);
                    }
                }
            }
        }
    }
    /* close file */
    fclose(fd);
    return 0;
}

int main(int argc, char**argv)
{
    //int ret;
    char opt, operation;
    char *dbFilename;
    char *logFilename;

    while ((opt = getopt(argc, argv, "hd:c:D:v:s:")) != -1) {
        switch (opt) {
            case 'h':
                printf("Usage:\n");
                printf("\t-h to display this help.\n");
                printf("\t-d <dbfile> <statfile>\n\t\tto create dbfile and fill it with the content statfile, a SURICATA stats log file.\n");
                printf("\t-c <dbfile>\n\t\tto create an empty dbfile.\n");
                printf("\t-D <dbfile>\n\t\tto delete the file dbfile.\n");
                printf("\t-v <dbfile>\n\t\tto get the information about data in dbfile.\n");
                printf("\t-s <dbfile>\n\t\tto get statistics about counters in dbfile.\n");
                break;
            case 'd':
                /* optarg contains the database filename */
                dbFilename = strdup(optarg);
                if (argc == 4) {
                    struct counterList * clist;
                    struct runList * rlist;

                    logFilename = strdup(argv[3]);
                    /* create counter and run lists */
                    clist = counterListCreate();
                    rlist = runListCreate();
                    /* parse log file to populate lists */
                    ParseLogFile(logFilename, clist, rlist);
                    /* DB creation with lists' content */
                    dbCreate(dbFilename, clist, rlist);
                    /* delete list */
                    counterListDelete(clist);
                    runListDelete(rlist);
                    free(dbFilename);
                    free(logFilename);
                }
                break;
            case 'c':
                /* optarg contains the database filename */
                dbFilename = strdup(optarg);
                /* exist? */
                /* create empty db */
                dbCreate(dbFilename, NULL, NULL);
                break;
            case 'D':
                /* optarg contains the database filename */
                dbFilename = strdup(optarg);
                /* exist? */
                if(remove(dbFilename) == 0)
                    printf("File %s deleted.\n", dbFilename);
                else
                    fprintf(stderr, "Error deleting the file %s.\n", dbFilename);
                break;
            case 'v':
                /* optarg contains the database filename */
                dbFilename = strdup(optarg);
                operation = opt;
                break;
            case 's':
                /* optarg contains the database filename */
                dbFilename = strdup(optarg);
                operation = opt;
                break;
            default:
                break;
        }
    }
    return 0;
}
