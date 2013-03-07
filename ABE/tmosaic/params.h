#ifndef PARAMS_H
#define PARAMS_H

#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
#include <cstdio>
#include <cctype>

using namespace std;

class params {
    public:
        map<string,string> pmap;
        string value(string key);
        long ivalue(string key);
        double dvalue(string key);
        void set(string key, string value);
        void read_file(char *file);
        params(char *file);
        params();
};

#endif
