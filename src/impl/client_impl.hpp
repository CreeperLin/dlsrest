#ifndef _H_DLS_CLIENT_IMPL_
#define _H_DLS_CLIENT_IMPL_
#include <map>
#include <string>
#include "../server/server.hpp"
using namespace std;

int acquire_lock_impl(string const &addr, string const &res_id, string const &cli_id, string const &auth, string &msg);
int release_lock_impl(string const &addr, string const &res_id, string const &cli_id, string const &auth, string &msg);
int stat_lock_impl(string const &addr, string const &res_id, string &stat, string &msg);
int list_lock_impl(string const &addr, map<string, Lock> &map, string &msg);
int hbeat_impl(string const &addr, string const &self_addr, string &msg);
int auth_impl(string const &addr, string &auth, string &msg);
void destory_clients_impl();
#endif
