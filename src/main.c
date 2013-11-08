#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  /* getopt() */
#include <string.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <limits.h> /* INT_MAX */

//#define DEBUG

#include "chi_debug.h"
#include "counter.h"
#include "run.h"
#include "dbmgr.h"


/*
 * 
 * name: ParseLogFile
 *        This function parses a SURICATA stats log file to fill a list with runs
 *        and another list with conters.
 * @param filename
 *        SURICATA stats log filename
 * @param clist
 *        pointer to counters'list
 * @param rlist
 *        pointer to runs'list
 * @return
 *        nothing
 */

void ParseLogFile(char * filename, struct counterList * clist, struct runList * rlist)
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
        long long int  counter_value;
        
        ret = sscanf(line, "%31s | %31s | %lld\n", counter_name, TM_name, &counter_value);
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
    printf("Found %d counter(s) in %d run(s)\n", clist->count, rlist->count);
    /* close file */
    fclose(fd);
    return;
}

int main(int argc, char**argv)
{
    //int ret;
    char opt;
    char *dbFilename;
    char *logFilename;

    while ((opt = getopt(argc, argv, "hc:f:d:s:")) != -1) {
        switch (opt) {
            case 'h':
                printf("Usage:\n");
                printf("-h\n\tto display this help.\n");
                printf("-c <dbfile>\n\tto create an empty dbfile.\n");
                printf("-f <dbfile> <statfile>\n\tto fill dbfile with the content statfile, a SURICATA stats log file.\n");
                printf("-d <dbfile>\n\tto delete the file dbfile.\n");
                printf("-s <dbfile>\n\tto get statistics about counters in database dbfile.\n");
                //printf("-m <dbfile> <statfile>\n\tto monitor the modifications of <statfile>, a SURICATA stats log file,\n\tand to populate the database <dbfile>.\n");
                break;
            case 'f':
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
                    /* fill DB with lists'content */
                    dbFill(dbFilename, clist, rlist);
                    /* delete list */
                    counterListDelete(clist);
                    runListDelete(rlist);
                    free(dbFilename);
                    free(logFilename);
                }
                else {
                    printf("Argument missing, type 'suristats -h' for help.\n");
                }
                break;
            case 'c':
                /* optarg contains the database filename */
                /* create empty db */
                dbCreate(optarg);
                break;
            case 'd':
                /* optarg contains the database filename */
                if(remove(optarg) == 0)
                    printf("File %s deleted.\n", optarg);
                else
                    fprintf(stderr, "Error deleting the file %s.\n", optarg);
                break;
            case 's':
                /* optarg contains the database filename */
                /* DB processing for statistics extraction */
                dbStatPrint(optarg);
                break;
            default:
                break;
        }
    }
    return 0;
}
