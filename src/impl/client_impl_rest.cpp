#include <cpprest/http_client.h>
#include <cpprest/json.h>
#pragma comment(lib, "cpprest_2_10")

using namespace web;
using namespace web::http;
using namespace web::http::client;

#include <map>
#include <iostream>
#include "../common/utils.hpp"
#include "../server/server.hpp"
#include "def.hpp"
using namespace std;

map<string, http_client *> client_map;

pplx::task<http_response> make_task_request(
    http_client &client,
    method mtd,
    string const &path,
    json::value const &jvalue)
{
    return (mtd == methods::GET || mtd == methods::HEAD) ? client.request(mtd, path) : client.request(mtd, path, jvalue);
}

http_client &get_client(string const &address)
{
    if (client_map.find(address) == client_map.end())
    {
        client_map[address] = new http_client(U(address));
    }
    return *client_map[address];
}

void make_request(
    string const &address,
    method mtd,
    string const &path,
    json::value const &jvalue,
    json::value &ret)
{
    TRACE("cli: " + address + " " + mtd + " " + path + " " + jvalue.serialize());
    make_task_request(get_client(address), mtd, path, jvalue)
        .then([](http_response response)
              {
                  if (response.status_code() == status_codes::OK)
                  {
                      return response.extract_json();
                  }
                  return pplx::task_from_result(json::value());
              })
        .then([&ret](pplx::task<json::value> previousTask)
              {
                  try
                  {
                      ret = previousTask.get();
                  }
                  catch (http_exception const &e)
                  {
                      cout << e.what() << endl;
                      ret = json::value::object();
                      ret[STATUS_KEY] = 2;
                      ret["msg"] = json::value::string(e.what());
                  }
              })
        .wait();
    if (ret[STATUS_KEY] == 2)
    {
        TRACE("exc");
        http_client *cli = client_map[address];
        client_map.erase(address);
        delete cli;
    } else if (ret[STATUS_KEY].is_null())
    {
        TRACE("exc null");
        ret[STATUS_KEY] = 2;
        ret["msg"] = json::value::string("null response");
    }
    display_json(ret, "R: ");
}

int acquire_lock_impl(string const &addr, string const &res_id, string const &cli_id, string const &auth, string &msg)
{
    auto putvalue = json::value::object();
    putvalue["res_id"] = json::value::string(res_id);
    putvalue["cli_id"] = json::value::string(cli_id);
    putvalue["auth"] = auth.empty() ? json::value() : json::value::string(auth);
    json::value ret;
    make_request(addr, methods::PUT, LOCK_PATH, putvalue, ret);
    if (ret.at(STATUS_KEY) == 0)
    {
        return 0;
    }
    msg = ret["msg"].as_string();
    return 1;
}

int release_lock_impl(string const &addr, string const &res_id, string const &cli_id, string const &auth, string &msg)
{
    auto putvalue = json::value::object();
    putvalue["res_id"] = json::value::string(res_id);
    putvalue["cli_id"] = json::value::string(cli_id);
    putvalue["auth"] = auth.empty() ? json::value() : json::value::string(auth);
    json::value ret;
    make_request(addr, methods::DEL, LOCK_PATH, putvalue, ret);
    if (ret.at(STATUS_KEY) == 0)
    {
        return 0;
    }
    msg = ret["msg"].as_string();
    return 1;
}

int stat_lock_impl(string const &addr, string const &res_id, string &stat, string &msg)
{
    auto putvalue = json::value::object();
    json::value ret;
    putvalue["res_id"] = json::value::string(res_id);
    make_request(addr, methods::POST, LOCK_PATH, putvalue, ret);
    if (ret.at(STATUS_KEY) == 0)
    {
        stat = ret.at("stat").serialize();
        return 0;
    }
    msg = ret["msg"].as_string();
    return 1;
}

int list_lock_impl(string const &addr, map<string, Lock> &map, string &msg)
{
    json::value ret;
    make_request(addr, methods::GET, LOCK_PATH, json::value::null(), ret);
    if (ret.at(STATUS_KEY) == 0)
    {
        for (auto const &p : ret.at("map").as_object())
        {
            json::value const &val = p.second;
            Lock lock;
            lock.res_id = p.first;
            lock.cli_id = val.at("cli_id").as_string();
            lock.expiry = val.at("expiry").as_integer();
            lock.timestamp = val.at("timestamp").as_integer();
            map[p.first] = lock;
        }
        return 0;
    }
    msg = ret["msg"].as_string();
    return 1;
}

int hbeat_impl(string const &addr, string const &self_addr, string &msg)
{
    auto putvalue = json::value::object();
    json::value ret;
    putvalue["addr"] = json::value::string(self_addr);
    make_request(addr, methods::POST, HBEAT_PATH, putvalue, ret);
    if (ret.at(STATUS_KEY) == 0)
    {
        return 0;
    }
    msg = ret["msg"].as_string();
    return 1;
}

int auth_impl(string const &addr, string &auth, string &msg)
{
    auto putvalue = json::value::object();
    json::value ret;
    make_request(addr, methods::POST, AUTH_PATH, putvalue, ret);
    if (ret.at(STATUS_KEY) == 0)
    {
        auth = ret["auth"].as_string();
        return 0;
    }
    msg = ret["msg"].as_string();
    return 1;
}

void destory_clients_impl()
{
    for (auto const &p : client_map)
    {
        delete p.second;
    }
}
