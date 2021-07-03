#include <random>
#include <sstream>
#include <iomanip>
#include "utils.hpp"
using namespace std;
using namespace web;

void display_json(json::value const &jvalue, string const &prefix)
{
    cout << prefix << jvalue.serialize() << endl;
}

string get_random_hex(int length)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    std::stringstream ss;
    for (auto i = 0; i < length / 2; i++)
    {
        ss << std::setfill('0') << std::setw(2) << std::hex << dis(gen);
    }
    return ss.str();
}
