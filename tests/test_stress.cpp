#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include <cassert>
#include "../src/client/client.hpp"
using namespace std;

string const SERVER_ADDR = "http://127.0.0.1:8888";
int const NUM_RES = 10;
int const MAX_SLEEP = 2000;
int const NUM_THREADS = 10;
int const NUM_REP = 3;
int res_val[NUM_RES] = {0};
string list_res[NUM_RES];
thread list_threads[NUM_THREADS];

void rand_sleep()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % MAX_SLEEP));
}

void test(int tid)
{
    for (int i = 0; i < NUM_REP; ++i)
    {
        rand_sleep();
        int res_id = rand() % NUM_RES;
        string const &res = list_res[res_id];
        int val = rand();
        cout << "thread: " << tid << " res: " << res << " val: " << val << endl;
        Client cli;
        init(cli, SERVER_ADDR);
        acquire(cli, res);
        res_val[res_id] = val;
        rand_sleep();
        string stats;
        int ret = stat(cli, res, stats);
        if (ret)
            cout << "stat error" << endl;
        rand_sleep();
        int val_2 = res_val[res_id];
        if (val_2 != val) {
            cout << "assert failed" << tid << " " << res_id << " " << val << " " << val_2 << endl;
            assert(0);
        }
        if (rand() % 2)
            release(cli, res);
    }
}

int main()
{
    for (int i = 0; i < NUM_RES; ++i)
    {
        list_res[i] = "/res_" + to_string(i);
    }
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        list_threads[i] = thread(test, i);
    }
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        list_threads[i].join();
    }
    destroy();
    return 0;
}
