#ifndef _H_DLS_UTILS_
#define _H_DLS_UTILS_
#include <cpprest/json.h>
#include <iostream>
using namespace std;
using namespace web;

#define TRACE(msg) cout << msg << "\n"

void display_json(json::value const &jvalue, string const &prefix);
string get_random_hex(int length);
#endif
