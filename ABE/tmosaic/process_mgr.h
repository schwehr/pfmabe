#ifndef PROCESS_MGR_H
#define PROCESS_MGR_H

#include <string>
#include <map>
#include <list>
#include <vector>
#include <cstdio>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>

using namespace std;

#define MSG_SIZE 32
#define MAX_PROCS 64


class process_env;

enum REQ { EXIT, WAIT, TASK, REPORT, SHUTDOWN };

class process_event {
    public:
    static process_env *ppe;
    string name;
    int value;
    bool ready;
    list<int> waiting;
    void wakeup();
};

typedef list<process_event *> task_list;

class process_env {
    public:
    int children;
    int to_parent[MAX_PROCS][2];
    int to_child[MAX_PROCS][2];
    int pipe_in;
    int pipe_out;
    int master_pipe[2];
    int must_quit;
    map<string,process_event *> sb;
    map<string,task_list> task_map;
    int next;
    int mpid;
    int mypid;
    string event_key ( string event_name, int value );
    void add_task ( string task_name, int value );
    int fetch_task ( string task_name );
    int pipe_read ( int fd, char *s, int size );
    int pipe_write ( int fd, char *s, int size );
    void launch ( void p() );
    void enqueue ( string event_name, int value, int pid );
    void complete ( string event_name, int value );
    bool ready ( string event_name, int value );
    bool wait_for ( string event_name, int value );
    int  get_task ( string name );
    void report_complete ( string name, int value );
    void send_exit();
    void send_shutdown();
    void shutdown();
    int receive ( char *s, int pid=0 );
    int send ( char *s, int pid=-1 );
    void run();
    process_env() {
	children=0;
        process_event::ppe = this;
        must_quit = 0;
        if ( pipe(master_pipe) ) {
            perror("master pipe create");
            exit(1);
        }
	mypid=-1;
        pipe_in = master_pipe[0];
    }
    ~process_env() {
    }
};

void log ( char *fmt, ... );
void log_start();
void dump_log();
#endif
