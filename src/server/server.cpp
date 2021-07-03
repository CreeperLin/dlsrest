#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include "../impl/client_impl.hpp"
#include "../impl/server_impl.hpp"
#include "../common/utils.hpp"
#include "../common/def.hpp"
#include "server.hpp"
using namespace std;

int const HBEAT_INTV = 10;
int const REP_HBEAT_TIMEOUT = HBEAT_INTV * 2;
int const LEADER_HBEAT_TIMEOUT = HBEAT_INTV * 2;

map<string, Lock> lock_map;
map<string, int> replicas;
int is_leader = 0;
int is_cand = 0;
string leader_auth;
string leader_addr;
string self_addr;
int leader_last_hbeat = 0;

void acquire_lock(string const &res_id, string const &cli_id, string const &auth)
{
    if (!auth.empty())
    {
        if (auth != get_leader_auth())
            throw runtime_error("invalid auth");
    }
    else if (!is_leader)
    {
        string msg;
        if (acquire_lock_impl(leader_addr, res_id, cli_id, string(), msg))
        {
            throw runtime_error("leader error: " + msg);
        }
        return;
    }
    auto it = lock_map.find(res_id);
    if (it != lock_map.end() && time(nullptr) - it->second.timestamp < it->second.expiry)
        throw runtime_error("locked: " + it->second.cli_id);
    Lock lock;
    lock.res_id = res_id;
    lock.cli_id = cli_id;
    lock.timestamp = time(nullptr);
    lock.expiry = DEFAULT_LOCK_EXPIRY;
    lock_map[res_id] = lock;
    if (!is_leader)
        return;
    string msg;
    for (auto const &p : replicas)
    {
        if (acquire_lock_impl(p.first, res_id, cli_id, leader_auth, msg))
        {
            TRACE("replica error: " + msg);
        }
    }
}

void release_lock(string const &res_id, string const &cli_id, string const &auth)
{
    if (!auth.empty())
    {
        if (auth != get_leader_auth())
            throw runtime_error("invalid auth");
    }
    else if (!is_leader)
    {
        string msg;
        if (release_lock_impl(leader_addr, res_id, cli_id, string(), msg))
        {
            throw runtime_error("leader error: " + msg);
        }
        return;
    }
    auto it = lock_map.find(res_id);
    if (it == lock_map.end())
        throw runtime_error("not locked");
    lock_map.erase(it);
    if (!is_leader)
        return;
    string msg;
    for (auto const &p : replicas)
    {
        if (release_lock_impl(p.first, res_id, cli_id, leader_auth, msg))
        {
            TRACE("replica error: " + msg);
        }
    }
}

Lock const & stat_lock(string const &res_id)
{
    auto it = lock_map.find(res_id);
    if (it == lock_map.end())
        throw runtime_error("not locked");
    return it->second;
}

int hbeat(string const &address)
{
    if (is_leader)
    {
        if (replicas.find(address) != replicas.end())
            return 0;
        int ret = replicas.empty() ? 1 : 0;
        replicas[address] = time(nullptr);
        return ret;
    }
    else
    {
        leader_last_hbeat = time(nullptr);
        return 0;
    }
}

map<string, Lock> const &get_map()
{
    return lock_map;
}

string const & get_leader()
{
    return leader_addr;
}

void set_leader(string const &address)
{
    leader_addr = address;
}

string new_leader_auth()
{
    return get_random_hex(8);
}

string const &get_leader_auth()
{
    if (leader_auth.empty() && !is_leader)
    {
        string msg;
        if (auth_impl(leader_addr, leader_auth, msg))
        {
            TRACE("auth error: " + msg);
        }
    }
    return leader_auth;
}

void elect_leader()
{
    TRACE("elected");
    is_leader = 1;
    
}

void init(string const &addr, string const &leader_address)
{
    self_addr = addr;
    if (leader_address.empty())
    {
        is_leader = 1;
        leader_auth = new_leader_auth();
    }
    else
    {
        is_leader = 0;
        leader_addr = leader_address;
        get_leader_auth();
    }
    TRACE("init: leader: " + to_string(is_leader) + " auth: " + leader_auth);
    lock_map.clear();
    if (is_leader)
    {
    }
    else
    {
        string msg;
        list_lock_impl(leader_addr, lock_map, msg);
    }
}

void thread_heartbeat()
{
    string msg;
    while (1)
    {
        if (is_leader)
        {
            for (auto it = replicas.cbegin(); it != replicas.cend(); /* no increment */)
            {
                if (hbeat_impl(it->first, self_addr, msg) && (time(nullptr) - it->second) > REP_HBEAT_TIMEOUT)
                {
                    TRACE("replica down: " + it->first);
                    replicas.erase(it++);
                }
                else
                {
                    ++it;
                }
            }
        }
        else
        {
            if (hbeat_impl(leader_addr, self_addr, msg) && (time(nullptr) - leader_last_hbeat) > LEADER_HBEAT_TIMEOUT)
            {
                TRACE("leader down");
                if (is_cand) elect_leader();
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(HBEAT_INTV));
    }
}

void run()
{
    thread th_hbeat(thread_heartbeat);
    thread th_impl(run_impl, self_addr);
    th_hbeat.join();
    th_impl.join();
    destory_clients_impl();
}
