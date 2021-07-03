#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#pragma comment(lib, "cpprest_2_10")

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <thread>
#include <string>
#include "../server/server.hpp"
#include "../common/utils.hpp"
#include "def.hpp"
using namespace std;

void handle_lock_get(http_request request)
{
    TRACE("handle GET");

    auto answer = json::value::object();
    auto json_map = json::value::object();

    auto map = get_map();
    for (auto const &p : map)
    {
        auto val = json::value::object();
        val["cli_id"] = json::value::string(p.second.cli_id);
        val["expiry"] = json::value::number(p.second.expiry);
        val["timestamp"] = json::value::number(p.second.timestamp);
        json_map[p.first] = val;
    }
    answer[STATUS_KEY] = 0;
    answer["map"] = json_map;

    display_json(json::value::null(), "R: ");
    display_json(answer, "S: ");

    request.reply(status_codes::OK, answer);
}

void handle_request(
    http_request request,
    function<void(json::value const &, json::value &)> action)
{
    auto answer = json::value::object();

    request
        .extract_json()
        .then([&answer, &action](pplx::task<json::value> task)
              {
                  try
                  {
                      auto const &jvalue = task.get();
                      display_json(jvalue, "R: ");

                      if (!jvalue.is_null())
                      {
                          action(jvalue, answer);
                      }
                  }
                  catch (http_exception const &e)
                  {
                      cout << e.what() << endl;
                  }
              })
        .wait();

    display_json(answer, "S: ");

    request.reply(status_codes::OK, answer);
}

void handle_lock_post(http_request request)
{
    TRACE("handle POST");

    handle_request(
        request,
        [](json::value const &jvalue, json::value &answer)
        {
            int status = 0;
            try
            {
                Lock const & lck = stat_lock(jvalue.at("res_id").as_string());
                auto val = json::value::object();
                val["res_id"] = json::value::string(lck.res_id);
                val["cli_id"] = json::value::string(lck.cli_id);
                val["type"] = json::value::number(lck.type);
                val["expiry"] = json::value::number(lck.expiry);
                val["ts"] = json::value::number(lck.timestamp);
                answer["stat"] = val;
            }
            catch (runtime_error const &e)
            {
                answer["msg"] = json::value::string(e.what());
                status = 1;
            }
            answer[STATUS_KEY] = status;
        });
}

void handle_lock_put(http_request request)
{
    TRACE("handle PUT");

    handle_request(
        request,
        [](json::value const &jvalue, json::value &answer)
        {
            int status = 0;
            string const &auth = jvalue.at("auth").is_null() ? string() : jvalue.at("auth").as_string();
            try
            {
                acquire_lock(jvalue.at("res_id").as_string(), jvalue.at("cli_id").as_string(), auth);
            }
            catch (runtime_error const &e)
            {
                answer["msg"] = json::value::string(e.what());
                status = 1;
            }
            answer[STATUS_KEY] = status;
        });
}

void handle_lock_del(http_request request)
{
    TRACE("handle DEL");

    handle_request(
        request,
        [](json::value const &jvalue, json::value &answer)
        {
            int status = 0;
            string const &auth = jvalue.at("auth").is_null() ? string() : jvalue.at("auth").as_string();
            try
            {
                release_lock(jvalue.at("res_id").as_string(), jvalue.at("cli_id").as_string(), auth);
            }
            catch (runtime_error const &e)
            {
                answer["msg"] = json::value::string(e.what());
                status = 1;
            }
            answer[STATUS_KEY] = status;
        });
}

void handle_hbeat_post(http_request request)
{
    TRACE("handle HBEAT");
    handle_request(
        request,
        [](json::value const &jvalue, json::value &answer)
        {
            int ret = hbeat(jvalue.at("addr").as_string());
            answer[STATUS_KEY] = 0;
            if (ret)
                answer["ret"] = 1;
        });
}

void handle_auth_post(http_request request)
{
    TRACE("handle auth");
    handle_request(
        request,
        [](json::value const &jvalue, json::value &answer)
        {
            string const &auth = get_leader_auth();
            answer[STATUS_KEY] = 0;
            answer["auth"] = json::value::string(auth);
        });
}

void run_impl(string const &address)
{
    http_listener lis_lock(address + LOCK_PATH);
    lis_lock.support(methods::GET, handle_lock_get);
    lis_lock.support(methods::POST, handle_lock_post);
    lis_lock.support(methods::PUT, handle_lock_put);
    lis_lock.support(methods::DEL, handle_lock_del);

    http_listener lis_hbeat(address + HBEAT_PATH);
    lis_hbeat.support(methods::POST, handle_hbeat_post);

    http_listener lis_auth(address + AUTH_PATH);
    lis_auth.support(methods::POST, handle_auth_post);

    lis_lock.open().wait();
    lis_hbeat.open().wait();
    lis_auth.open().wait();

    while (1)
        std::this_thread::sleep_for(std::chrono::seconds(1));
}
