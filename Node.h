//
// Created by Jeroen Smienk on 06-03-17.
//

#ifndef LORAP2PNETWORK_NODE_H
#define LORAP2PNETWORK_NODE_H

#include <netinet/in.h>

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

    inline in_addr getAddress()
    {
        return addr;
    };

    bool send();

    inline std::string getNickname()
    {
        return nickname;
    };
};

#endif //LORAP2PNETWORK_NODE_H