#include "params.h"

string params::value(string key)
{
    if ( pmap.count(key) < 1 ) return "";
    return pmap[key];
}

double params::dvalue(string key)
{
    string v;

    v = value(key);
    if ( v == "" ) return 0.0;
    return atof(v.c_str());
}

long params::ivalue(string key)
{
    string v;

    v = value(key);
    if ( v == "" ) return 0;
    return atol(v.c_str());
}

void params::set(string key, string value)
{
    pmap[key] = value;
}

void params::read_file(char *file)
{
    FILE *fp;
    char s[1000];
    char *p;
    int line, n;


    fp = fopen ( file, "r" );
    if ( ! fp ) {
        fprintf(stderr,"Could not open parameter file: %s\n", file );
        exit(1);
    }

    line = 0;
    while ( fgets(s,1000,fp) ) {
        line++;
        if ( s[0] == '#' || isspace(s[0]) ) continue;
        n = strlen(s);
        if ( s[n-1] == '\n' ) {
            s[n-1] = 0;
            n--;
        }
        p = strchr(s,':');
        if ( !p ) {
            fprintf(stderr,"Error in params line %d: %s", s );
            continue;
        }
        *p = 0;
        p++;
        while ( *p and isspace(*p) ) p++;
        set ( s, p );
    }
    fclose ( fp );
}

params::params(char *file)
{
    read_file ( file );
}

params::params()
{
}
