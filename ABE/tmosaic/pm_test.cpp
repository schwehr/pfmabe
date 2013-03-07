#include <cstdio>
#include "process_mgr.h"

process_env pe;

void start()
{
    int task;
    printf("Start process %d\n", getpid() );
    while ( (task = pe.get_task("start")) >= 0 ) {
        usleep(rand()%1000);
        pe.report_complete ( "work", task );
        printf("Process %d, completed %d\n", getpid(), task );
    }
}

void append()
{
    int task;
    printf("Append process %d\n", getpid() );
    while ( (task = pe.get_task("append")) >= 0 ) {
        pe.wait_for("work",task);
        usleep(rand()%1000);
        pe.report_complete ( "append", task );
        printf("Process %d, appended %d\n", getpid(), task );
    }
}

int main ( int argc, char **argv )
{
    int i, nkids, msgs;

    nkids = argc > 1 ? atoi(argv[1]) : 2;
    msgs = argc > 2 ? atoi(argv[2]) : 5;

    for ( i = 0; i < nkids*msgs; i++ ) {
        pe.add_task("start",i);
        pe.add_task("append",i);
    }

    for ( i = 0; i < nkids; i++ ) {
        pe.launch ( start );
        pe.launch ( append );
    }

    pe.run();
    return 0;
}
