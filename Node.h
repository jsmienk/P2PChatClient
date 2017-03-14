//
// Created by Jeroen Smienk on 06-03-17.
//

#ifndef LORAP2PNETWORK_NODE_H
#define LORAP2PNETWORK_NODE_H

#ifdef __WIN32__
#include <inaddr.h>
//#include <wininet.h>
#else
#include <netinet/in.h>

#endif

#include <string>
#include <vector>


#include "cryptopp565/rsa.h"

class Node
{
    // ip
    in_addr addr;

    // public key
    CryptoPP::RSA::PublicKey Kp;

    // nick name (optional; not used in node)
    std::string nickname;

    // list of nodes that can reach this node
    std::vector<Node> reached_by;

public:
    Node();

    inline in_addr get_address()
    {
        return addr;
    };

    bool redirect();

    std::string decode();

    inline std::string get_nickname()
    {
        return nickname;
    };
};

#endif //LORAP2PNETWORK_NODE_H
