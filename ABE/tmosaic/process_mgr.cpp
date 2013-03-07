#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include "process_mgr.h"

using namespace std;

char log_data[1000000][80];
int log_ct;
int start_time;
timeval now;

process_env *process_event::ppe;
void log_start()
{
    gettimeofday ( &now, NULL );
    start_time = now.tv_sec*1000000 + now.tv_usec;
}

void log ( char *fmt, ... )
{
    va_list ap;
    va_start ( ap, fmt );

    gettimeofday ( &now, NULL );
    sprintf(log_data[log_ct], "%12d ", now.tv_sec*1000000 + now.tv_usec - start_time );
    vsprintf(log_data[log_ct]+13,fmt,ap);
    log_ct++;
}

void dump_log()
{
    int i;
    for ( i = 0; i < log_ct; i++ ) printf("%s\n",log_data[i]);
}

int process_env::pipe_read ( int fd, char *s, int size)
{
    int n = 0, left;

    left = size;
    while ( left > 0 ) {
       n = read ( fd, s, left );
       if ( n < 0 ) return n;
       s += n;
       left -= n;
    }
    return size;
}

int process_env::pipe_write ( int fd, char *s, int size)
{
    int n = 0, left;

    left = size;
    while ( left > 0 ) {
       n = write ( fd, s, left );
       if ( n < 0 ) return n;
       s += n;
       left -= n;
    }
    return size;
}

void process_event::wakeup()
{
    int t;
    while ( waiting.size() > 0 ) {
        t = waiting.front();
        waiting.pop_front();
        log ("wakeup %d",t);
        t = ppe->send ( "1", t );
        if ( t < 0 ) perror("wakeup send");
    }
}

string process_env::event_key ( string event_name, int value )
{
    char s[80];
    string ek;

    sprintf(s,"%s-%d", event_name.c_str(), value );
    ek = s;
    return ek;
}

int process_env::receive ( char *s, int pid )
{
    int t, child;

    //printf("Process %d receiving pid %d\n", mypid, pid );
    if ( mypid >= 0 ) {
        t = pipe_read ( pipe_in, s, MSG_SIZE );
    } else {
        t = read ( master_pipe[0], s, 1 );
        if ( t != 1 ) {
            perror("master pipe read");
            exit(1);
        }
        child = s[0];
        t = pipe_read ( to_parent[child][0], s, MSG_SIZE );
    }
        
    return t;
}

int process_env::send ( char *s, int pid )
{
    int t;
    char msg[MSG_SIZE];

    //printf("Sending %s to %d\n", s, pid );
    if ( mypid >= 0 ) {
        msg[0] = mypid;
        write ( master_pipe[1], msg, 1 );
    } else {
        pipe_out = to_child[pid][1];
    }
    strcpy ( msg, s );
    t = pipe_write ( pipe_out, msg, MSG_SIZE );
    //printf("Sent %s to %d\n", s, pid );
    return t;
}

//
//  Used by the master to add a task to a task queue
//
void process_env::add_task ( string task_name, int value )
{
    process_event *p;
    p = new process_event();
    p->name = task_name;
    p->value = value;
    task_map[task_name].push_back(p);
}

//
//  Used by the master to fetch a task from a task queue
//
int process_env::fetch_task ( string task_name )
{
    process_event *p;
    int value;
    if ( task_map[task_name].size() < 1 ) return -1;
    p = task_map[task_name].front();
    task_map[task_name].pop_front();
    value = p->value;
    delete p;
    return value;
}

//
//  Used by the master to add a process to an event queue
//
void process_env::enqueue ( string event_name, int value, int pid )
{
    process_event *p;
    string k;
    k = event_key(event_name,value);
    if ( sb.count(k) == 0 ) {
        p = new process_event();
        p->name = event_name;
        p->value = value;
        p->ready = false;
        sb[k] = p;
    }
    p = sb[k];
    p->waiting.push_back(pid);
}

//
//  Used by the master to record the completion of an event and to
//  wake the processes on the event queue.
//
void process_env::complete ( string event_name, int value )
{
    process_event *p;
    string k;
    k = event_key(event_name,value);
    if ( sb.count(k) == 0 ) {
        p = new process_event();
        p->name = event_name;
        p->value = value;
        p->ready = true;
        sb[k] = p;
    } else {
        p = sb[k];
        p->ready = true;
        p->wakeup();
    }
}

//
//  Used by the master to check the status of an event
//
bool process_env::ready ( string event_name, int value )
{
    string k;
    k = event_key(event_name,value);
    if ( sb.count(k) == 0 ) return false;
    return sb[k]->ready;
}

void process_env::send_exit()
{
    int t;
    char s[80];
    sprintf(s,"%d %d", EXIT, mypid );
    t = send ( s );
    if ( t < 0 ) perror("exit send");
}

void process_env::send_shutdown()
{
    int t;
    char s[80];
    sprintf(s,"%d %d", SHUTDOWN, mypid );
    t = send ( s );
    if ( t < 0 ) perror("shutdown send");
}

void process_env::shutdown()
{
    int t;
    char s[80];
    process_event *e;
    map<string,process_event *>::iterator i = sb.begin();

    strcpy(s,"-1");
    while ( i != sb.end() ) {
        e = i->second;
        list<int>::iterator j = e->waiting.begin();
        while ( j != e->waiting.end() ) {
            t = *j;
            send ( s, t );
            j++;
        }
        i++;
    }
}

//
//  Used by a worker to wait for the completion of an event.
//  This could result in an immediate reply or the process
//  could be placed in an event queue.
//
bool process_env::wait_for ( string event_name, int value )
{
    int t;
    char s[80];
    sprintf(s,"%d %d %s %d", WAIT, mypid, event_name.c_str(), value );
    t = send ( s );
    if ( t < 0 ) perror("wait send");
    t = receive ( s );
    if ( t > 0 ) sscanf(s,"%d",&t);
    return t;
}

//
//  Used by a worker to get a task to work on.
//  The worker sends the name of the task and the reply
//  is an integer: -1 means no more work to do.
//
int  process_env::get_task ( string name )
{
    int t;
    char s[80];
    sprintf(s,"%d %d %s 0", TASK, mypid, name.c_str() );
    t = send ( s );
    if ( t < 0 ) {
        perror("task send");
        return -1;
    }
    t = receive ( s );
    if ( t < 0 ) {
        perror("task recv");
        return -1;
    }
    sscanf(s,"%d",&t);
    return t;
}

//
//  Used by a worker to report that a task has been completed.
//
void process_env::report_complete ( string name, int value )
{
    int t;
    char s[80];

    sprintf(s,"%d %d %s %d", REPORT, mypid, name.c_str(), value );
    t = send ( s );
    if ( t < 0 ) perror("wait send");
}

void process_env::launch ( void p() )
{
    int pid;

    if ( pipe(to_parent[children]) ) {
        perror("to_parent pipe create");
        exit(1);
    }
    if ( pipe(to_child[children]) ) {
        perror("to_parent pipe create");
        exit(1);
    }

    pid = fork();
    if ( pid == 0 ) {
        //
        //      Child
        //
        mypid = children;
        pipe_in = to_child[mypid][0];
        pipe_out = to_parent[mypid][1];
	close ( to_child[mypid][1] );
	close ( to_parent[mypid][0] );
	close ( master_pipe[0] );
        p();
        send_exit();
       exit(0);
    } else {
        //
        //      Parent
        //
	close ( to_child[children][0] );
	close ( to_parent[children][1] );
        children++;
    }
}

void process_env::run()
{
    int t, task = 0;
    char s[80];
    int req, pid, value;
    char task_name[16];

    while ( children > 0 ) {
        receive ( s, 1 );
        sscanf(s,"%d %d %s %d", &req, &pid, task_name, &value );
        switch ( req ) {
            case EXIT:
                children--;
            case WAIT:
                if ( ready(task_name,value) ) {
                    t = send ( "1", pid );
                } else {
                    enqueue ( task_name, value, pid );
                }
                break;
            case TASK:
                task = fetch_task(task_name);
                sprintf(s,"%d",task);
                t = send ( s, pid );
                break;
            case REPORT:
                complete ( task_name, value );
                break;
        }
    }
}
