#include <iostream>

#include "P2PClient.h"

using namespace std;

int main(int argc, char *argv[]) {
    P2PClient c;
    string host;

    cout << "Enter hostname: ";
    cin >> host;

    //connect to host
    c.conn(host, 80);

    //send some data
    c.sendData("GET / HTTP/1.1\r\n\r\n");

    //receive and echo reply
    cout << "----------------------------\n\n";
    cout << c.receive(1024);
    cout << "\n\n----------------------------\n\n";

    //done
    return 0;
}