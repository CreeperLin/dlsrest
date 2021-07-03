#include <iostream>
#include <string>
#include <map>
#include "../server/server.hpp"
using namespace std;

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cout << "Usage: server http://ip:port [leader_addr]" << endl;
        return 1;
    }
    string leader_addr = argc > 2 ? argv[2] : string();
    string self_addr = argv[1];
    init(self_addr, leader_addr);
    run();
    return 0;
}
