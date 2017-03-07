//
// Created by Jeroen Smienk on 02-03-17.
//

#ifndef LORAP2PNETWORK_P2PROUTER_H
#define LORAP2PNETWORK_P2PROUTER_H

#ifdef __WIN32__

# include <winsock2.h>

#else

#include <sys/socket.h>
#include <arpa/inet.h>

#endif

#include <netdb.h>

#include <string>
#include <map>
#include "Node.h"

class P2PRouter {

    // table with every node known
    std::map<std::string, Node> routing_table;

    // socket
    int sock;

    // Bottom two are IP dependent. Will change when we use LoRa
    // my address
    std::string address;
    // the port we chose to operate on
    int port;

public:
    P2PRouter();

    void die(char *s);

    bool conn(std::string address, int port);

    bool sendData(std::string data, in_addr addr);

    bool broadcast(std::string data);

    std::string receive(int);
};

#endif //LORAP2PNETWORK_P2PROUTER_H
