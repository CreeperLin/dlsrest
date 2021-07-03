#ifndef _H_DLS_CLIENT_
#define _H_DLS_CLIENT_
#include <string>
using namespace std;

struct Client {
    string addr;
    string id;
};

int try_acquire(Client const & cli, string const &res_id);
int acquire(Client const & cli, string const &res_id);
int release(Client const & cli, string const &res_id);
int stat(Client const & cli, string const &res_id, string &stat);
void init(Client & cli, string const &addr);
void destroy();
#endif
