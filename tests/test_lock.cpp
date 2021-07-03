#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cassert>
#include "../src/client/client.hpp"
using namespace std;

string const LEADER_ADDR = "http://127.0.0.1:8890";
string const REP_ADDR = "http://127.0.0.1:8891";
string const RES_ID = "/res1";
string const RES2_ID = "/res2";

int main()
{
    string res_stat;
    Client cli;
    init(cli, LEADER_ADDR);
    cout << "cli: " << cli.id << endl;
    cout << "acquire" << endl;
    acquire(cli, RES_ID);
    acquire(cli, RES2_ID);
    cout << "stat" << endl;
    stat(cli, RES_ID, res_stat);
    cout << res_stat << endl;
    cout << "try acquire" << endl;
    assert(try_acquire(cli, RES_ID) == 1);
    cout << "release" << endl;
    release(cli, RES_ID);
    release(cli, RES_ID);
    destroy();

    init(cli, REP_ADDR);
    cout << "rep cli: " << cli.id << endl;
    cout << "acquire" << endl;
    acquire(cli, RES_ID);
    cout << "stat" << endl;
    stat(cli, RES_ID, res_stat);
    cout << res_stat << endl;
    cout << "try acquire" << endl;
    assert(try_acquire(cli, RES_ID) == 1);
    cout << "release" << endl;
    release(cli, RES_ID);
    release(cli, RES_ID);
    cout << "expire" << endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    cout << "acquire 2" << endl;
    acquire(cli, RES2_ID);
    cout << "stat 2" << endl;
    stat(cli, RES2_ID, res_stat);
    cout << res_stat << endl;
    cout << "release 2" << endl;
    release(cli, RES2_ID);
    destroy();
    return 0;
}
