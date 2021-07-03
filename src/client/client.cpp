#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <map>
#include "client.hpp"
#include "../impl/client_impl.hpp"
#include "../common/utils.hpp"
using namespace std;


int try_acquire(Client const & cli, string const &res_id)
{
    string msg;
    return acquire_lock_impl(cli.addr, res_id, cli.id, string(), msg);
}

int acquire(Client const & cli, string const &res_id)
{
    string msg;
    while (1)
    {
        if (!acquire_lock_impl(cli.addr, res_id, cli.id, string(), msg))
            break;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}

int release(Client const & cli, string const &res_id)
{
    string msg;
    return release_lock_impl(cli.addr, res_id, cli.id, string(), msg);
}

int stat(Client const & cli, string const &res_id, string &stat)
{
    string msg;
    return stat_lock_impl(cli.addr, res_id, stat, msg);
}

string new_client_id()
{
    return get_random_hex(8);
}

void init(Client &cli, string const &addr)
{
    cli.id = new_client_id();
    cli.addr = addr;
}

void destroy()
{
    destory_clients_impl();
}
