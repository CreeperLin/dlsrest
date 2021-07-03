#ifndef _H_DLS_SERVER_
#define _H_DLS_SERVER_

#include <string>
#include <map>
using namespace std;

struct Lock
{
    string res_id;
    string cli_id;
    int type = 0;
    int timestamp = 0;
    int expiry = 0;
};

void acquire_lock(string const &res_id, string const &cli_id, string const &cred);
void release_lock(string const &res_id, string const &cli_id, string const &cred);
Lock const & stat_lock(string const &res_id);
int hbeat(string const &address);
map<string, Lock> const &get_map();
string const & get_leader();
void set_leader(string const &address);
string const &get_leader_auth();
void init(string const &addr, string const &leader_address);
void run();
#endif
