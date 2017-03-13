/**
 * Created by Jeroen Smienk on 02-03-17.
 *
 * Receiving and sending Onions!
 * The upper peel is encrypted with this router's public key.
 * We send the peeled Onion to the address inside the upper peel.
 *
 * 1. Listen for messages.
 * 2. Decrypt peel with this router's public key.
 * 3. Check if the addressee is in the router's routing table.
 * 4.   yes: send the rest of the Onion to the addressee.
 *      no:  broadcast rest of the message                      TODO ?
 */

// include the right file depending on the OS
#ifdef __WIN32__

#include <winsock2.h>

#else
#include <arpa/inet.h>
#include <sys/socket.h>

#endif

#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <iostream>
#include "P2PRouter.h"

using namespace std;

int const BUFLEN = 512;
// port number = LORA
int const PORT = 5672;

P2PRouter::P2PRouter()
{
    sock = -1;
    port = 0;
    address = "";
}

void die(string err_message)
{
    cerr << err_message << endl;
    exit(1);
}

bool P2PRouter::broadcast(std::string data)
{
    return true;
}

//int main(int argc, char *argv[])
//{
//    struct sockaddr_in si_me, si_other;
//
//    int s, i, slen = sizeof(si_other), recv_len;
//    char buf[BUFLEN];
//
//    //create a UDP socket
//    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
//    {
//        die("socket");
//    }
//
//    // zero out the structure
//    memset((char *) &si_me, 0, sizeof(si_me));
//
//    si_me.sin_family = AF_INET;
//    si_me.sin_port = htons(PORT);
//    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
//
//    //bind socket to port
//    if (bind(s, (struct sockaddr *) &si_me, sizeof(si_me)) == false)
//    {
//        die("bind");
//    }
//
//    //keep listening for data
//    for (;;)
//    {
//        printf("Waiting for data...");
//        fflush(stdout);
//
//        //try to receive some data, this is a blocking call
//        if ((recv_len = (int) recvfrom(s, buf, (size_t) BUFLEN, 0, (struct sockaddr *) &si_other,
//                                       (socklen_t *) &slen)) == -1)
//        {
//            die("recvfrom()");
//            break;
//        }
//
//        //print details of the client/peer and the data received
//        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
//        printf("Data: %s\n", buf);
//
//        //now reply the client with the same data
//        if (sendto(s, buf, (size_t) recv_len, 0, (struct sockaddr *) &si_other, (socklen_t) slen) == -1)
//        {
//            die("sendto()");
//            break;
//        }
//    }
//
//    exit(1);
//}