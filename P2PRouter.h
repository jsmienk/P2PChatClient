//
// Created by Jeroen Smienk on 02-03-17.
//

#ifndef LORAP2PNETWORK_P2PROUTER_H
#define LORAP2PNETWORK_P2PROUTER_H

#ifdef __WIN32__

# include <winsock2.h>

#else

#include <sys/socket.h>

#endif

#include <arpa/inet.h>
#include <netdb.h>

#include <string>

class P2PRouter {
private:
    int sock;
    std::string address;
    int port;
    struct sockaddr_in server;

public:
    P2PRouter();

    bool conn(std::string address, int port);

    bool sendData(std::string data);

    bool broadcast(std::string data);

    std::string receive(int);
};

#endif //LORAP2PNETWORK_P2PROUTER_H
