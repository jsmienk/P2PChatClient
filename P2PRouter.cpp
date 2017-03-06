//
// Created by Jeroen Smienk on 02-03-17.
//

// include the right file depending on the OS
#ifdef __WIN32__

#include <winsock2.h>

#else

#include <sys/socket.h>

#endif

#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>


#include "P2PRouter.h"

using namespace std;

P2PRouter::P2PRouter()
{
    sock = -1;
    port = 0;
    address = "";
}

bool P2PRouter::broadcast(std::string data)
{
    return true;
}

/**
 * Connect to a host on a certain port number
 * @param address
 * @param port
 * @return
 */
bool P2PRouter::conn(string address, int port)
{
    if (WSAStartup(0x0101, &w) != 0)
    {
        fprintf(stderr, "Could not open Windows connection.\n");
        exit(0);
    }

    //create socket if it is not already created
    if (sock == -1)
    {
        // create the socket
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock == INVALID_SOCKET)
        {
            fprintf(stderr, "Could not create socket.\n");
            WSACleanup();
            exit(0);
        }

        cout << "Socket created.\n";
    }
    else
    {
        cerr << "Socket could not be created!" << endl;
        // socket could not be created
        return false;
    }

    //setup address structure
    if (inet_addr(address.c_str()) == -1)
    {
        struct hostent *he;
        struct in_addr **addr_list;

        //resolve the hostname, its not an ip address
        if ((he = gethostbyname(address.c_str())) == NULL)
        {
            //gethostbyname failed
            herror("gethostbyname");
            cout << "Failed to resolve hostname\n";

            return false;
        }

        //Cast the h_addr_list to in_addr , since h_addr_list also has the ip address in long format only
        addr_list = (struct in_addr **) he->h_addr_list;

        for (int i = 0; addr_list[i] != NULL; i++)
        {
            //strcpy(ip , inet_ntoa(*addr_list[i]) );
            server.sin_addr = *addr_list[i];

            cout << address << " resolved to " << inet_ntoa(*addr_list[i]) << endl;

            break;
        }
    }
        //plain ip address
    else
    {
        server.sin_addr.s_addr = inet_addr(address.c_str());
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    //Connect to remote server
    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    cout << "Connected\n";
    return true;
}

/**
 * Send data to the connected host
 * @param data
 * @return
 */
bool P2PRouter::sendData(string data)
{
    //Send some data
    if (send(sock, data.c_str(), strlen(data.c_str()), 0) < 0)
    {
        perror("Send failed : ");
        return false;
    }
    cout << "Data send\n";

    return true;
}

/**
 * Receive data from the connected host
 * @param size
 * @return
 */
string P2PRouter::receive(int size = 512)
{
    char buffer[size];
    string reply;

    //Receive a reply from the server
    if (recv(sock, buffer, sizeof(buffer), 0) < 0)
    {
        puts("recv failed");
    }

    reply = buffer;
    return reply;
}