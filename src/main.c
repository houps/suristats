#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  /* getopt() */
#include <string.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#include "counter.h"
#include "dbmgr.h"

/* TRACE MACRO */
#define TRACE() //printf()

#define CMD_FORMAT(s1,s2)   ("{\"%s\": \"%s\"}", (s1), (s2))
#define PCAP_FILE_ARG       "\"arguments\": {\"filename\":\"%s\",\"output-dir\":\"%s\"}"

struct counterList *  ParseLogFile(char * filename)
{
    FILE * fd;
    int i, ret;
    char line[128];
    struct counterList * list;
    int run = 0;

    /* open file */
    fd = fopen(filename, "r");
    if (fd == NULL) {
        perror("File opening error.");
        exit(EXIT_FAILURE);
    }
    /* create counter list */
    list = counterListCreate();
    while(fgets(line, 128, fd) != NULL) {

        char counter_name[80];
        char TM_name[32]; 
        long int  counter_value;
        int uptime = 0;

        ret = sscanf(line, "%31s | %31s | %ld\n", counter_name, TM_name, &counter_value);
        if (ret == 3) {
            struct counter * cnt = counterCreate(counter_name, TM_name, counter_value, run, uptime);
            counterListAppend(list, cnt);
        }
        else {
            /* read header (5 lines) */
            /* pour le moment on les ignore */
            for (i = 0; i < 4; i++) {
                char *c;

                c = fgets(line, 128, fd);
                if (c == NULL) {
                    perror("Corrupted file.");
                    exit(EXIT_FAILURE);
                }
                ret = sscanf(line, "Date: %d/%d/%d -- %d:%d:%d (uptime: %dd, %02dh %02dm %02ds)\n",
                             month, day, year,
                             hours, minutes, seconds,
                             uptime_days, uptime_hours, uptime_minutes, uptime_seconds);
                if (ret == 10) {
                    prev_uptime = uptime;
                    uptime = uptime_days * 24 * 3600 + uptime_hours * 3600 + uptime_minutes * 60 + uptime_seconds;
                    if (uptime <= prev_uptime) {
                        run++;
                    }
                }
            }
            //counterListDisplay(list);
        }
#if 0
-------------------------------------------------------------------
Date: 7/2/2013 -- 15:29:17 (uptime: 0d, 00h 00m 28s)
-------------------------------------------------------------------
Counter                   | TM Name                   | Value
-------------------------------------------------------------------
capture.kernel_packets    | AFPacketeth31             | 2511127
#endif
    }
    counterListDisplay(list);
    /* close file */
    fclose(fd);
    return list;
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
                printf("   -h to display this help.\n");
                printf("   -d <dbfile> <statfile> to populate dbfile with a SURICATA 'stats.log' file.\n");
                break;
            case 'd':
                /* optarg contains the database filename to be populated */
                dbFilename = strdup(optarg);
                operation = opt;
                break;
            default:
                break;
        }
    }
    if ((operation == 'd') && (argc == 4)) {

        struct counterList * list;

        logFilename = strdup(argv[3]);
        /* let's go */
        list = ParseLogFile(logFilename);
        if (list == NULL) {
            perror("Log file empty or corrupted.");
            exit(EXIT_FAILURE);
        }
        dbCreate(dbFilename, list);
        counterListDelete(list);
        free(dbFilename);
        free(logFilename);
    }
    return 0;
}
