#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <thread>
#include <ifaddrs.h>


#include <jsoncpp/dist/json/json.h>

#include <jsoncpp/include/json/reader.h>
#include <jsoncpp/include/json/writer.h>
#include <jsoncpp/include/json/value.h>

//#include "json.hpp"
//using json = nlohmann::json;


#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#ifdef __WIN32__
#include <winsock2.h>
#else

#include <sys/socket.h>

#endif

// port name == LoRa
#define PORT 5672
#define BLEN 512
#define MASK "255"

// pre-declarations
static std::string get_network_interface_address();

static void display_error(const char *on_what);

int sock;                       // socket
struct sockaddr_in socket_me;   // our socket address
struct sockaddr_in socket_them; // the socket address for the sender

int data_size;

std::string address = get_network_interface_address();
std::string address_broadcast;
std::string last_send_message;

void receiving()
{
    std::cout << "Receive thread started." << std::endl;

    // Wait for incoming messages
    // 3 types:
    //  - Response on initial broadcast
    //  - New node in the network
    //  - Onion we have to redirect
    char buffer[BLEN];              // receive buffer

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    for (;;)
    {
        memset(buffer, 0, sizeof(buffer));

        // Receive anything
        data_size = (int) recv(sock, buffer, BLEN, 0);
        // If we received something
        if (data_size < 0)
        {
            display_error("recvfrom(2)");
        }

        std::string received = buffer;

        // If the received message is not ours
//        if (received != last_send_message)
//        {
        // Print what we got
        std::cout << "Received: " << received << std::endl;


//        rapidjson::Document json;
//        json.Parse(received.c_str());

        // Check if parse succeeded
//        json j = json::parse(received.c_str());

            std::string strJson = received; // need escape the quotes

            Json::Value root;
            Json::Reader reader;
            bool parsingSuccessful = reader.parse( strJson.c_str(), root );     //parse process
            if ( !parsingSuccessful )
            {
                std::cout  << "Failed to parse"
                           << reader.getFormattedErrorMessages();
                display_error("Non Parsable");
            }
            std::cout << root.get("mykey", "A Default Value if not exists" ).asString() << std::endl;
//            return 0;
        }




        try
        {
            auto j1 = json::parse(received);
//            std::cout << (j1.type() == json::value_t::null) << std::endl;
            std::cout << j1.size()<< std::endl;
            if ((j1 == nullptr) !=0) {
            display_error("Non Parsable");
            }
            std::cout << j1 << std::endl;
        } catch (std::invalid_argument){

        }

//            char json_buffer[sizeof(received.c_str())];
//            memcpy(j1, received.c_str(), received.size());
//            if (json.ParseInsitu(json_buffer).HasParseError())
//            {
//                display_error("JSON could not be parsed!");
//            }
//        } catch (std::exception)
//        {
//
//        }

//
//        /*
//         * Send the formatted result back to the
//         * client program:
//         */
//        data_size = (int) sendto(sock,                             // Socket to send result
//                                 "Hallo?",                 // The datagram result to send
//                                 sizeof("Hallo?"),         // The datagram length
//                                 0,                                // Flags: no options
//                                 (struct sockaddr *) "145.76.241.255", // Address
//                                 (socklen_t) sizeof("145.76.241.255"));       // Client address length
//
//        // if the sending succeeded
//        if (data_size < 0)
//        {
//            display_error("sendto(2)");
//        }
//        }
    }
#pragma clang diagnostic pop
}

void sending()
{
    std::cout << "Send thread started." << std::endl;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    for (;;)
    {
        std::string console_input;
        getline(std::cin, console_input);

        std::cout << "Sending: " << console_input << std::endl;

        data_size = (int) sendto(sock, console_input.c_str(), console_input.size(), 0,
                                 (struct sockaddr *) &socket_them,
                                 sizeof(socket_them));
        last_send_message = console_input;
    }
#pragma clang diagnostic pop
}

int main(int argc, char **argv)
{

    // Prepare socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        display_error("socket()");
    }
    std::cout << "Socket prepared." << std::endl;

    // Create a socket address, for use with bind(2)
    memset(&socket_me, 0, sizeof(socket_me));
    socket_me.sin_family = AF_INET;
    socket_me.sin_port = htons(PORT);
    socket_me.sin_addr.s_addr = INADDR_ANY;

    // Bind a address to our socket, so that client programs can contact this node
    if ((data_size = bind(sock, (struct sockaddr *) &socket_me, (socklen_t) sizeof(socket_me))) == -1)
    {
        display_error("bind()");
    }
    std::cout << "Socket bound." << std::endl;

    // Broadcast socket
    socket_them.sin_family = AF_INET;
    socket_them.sin_port = htons(PORT);

    std::size_t last_point_pos = address.rfind(".");
    address_broadcast = address.substr(0, last_point_pos + 1) + MASK;
    std::cout << "Broadcast address formed: " << address_broadcast << std::endl;

    socket_them.sin_addr.s_addr = inet_addr(address_broadcast.c_str());
    if (socket_them.sin_addr.s_addr == INADDR_NONE)
    {
        display_error("bad address");
    }

    // Allow broadcasts
    if ((data_size = setsockopt(sock,
                                SOL_SOCKET,
                                SO_BROADCAST,
                                &socket_them,
                                sizeof(socket_them))) == -1)
    {
        display_error("setsockopt(SO_BROADCAST)");
    }

    // Broadcast my IP, Kp and request other IPs and Kps (and nickname if client)
    std::string broadcast_message = "{\"hello\":3}";
    data_size = (int) sendto(sock, broadcast_message.c_str(), broadcast_message.size(), 0,
                             (struct sockaddr *) &socket_them,
                             sizeof(socket_them));
    last_send_message = broadcast_message;

    // Start the thread that will receive messages
    std::thread thread_receive(receiving);
    // Start the thread that will send messages
    std::thread thread_send(sending);

    thread_receive.join();
    thread_send.join();

    // Close the socket and exit
    close(sock);
    return 0;
}

/**
 * Displays the error and reports back to the shell
 * @param on_what error message
 */
static void display_error(const char *on_what)
{
    fputs(strerror(errno), stderr);
    fputs(": ", stderr);
    fputs(on_what, stderr);
    fputc('\n', stderr);
    throw std::exception();
}

/**
 * Get a IPv4 network interface address
 * @return address or nullptr
 */
static std::string get_network_interface_address()
{
    struct ifaddrs *ifAddrStruct = NULL;
    struct ifaddrs *ifa = NULL;
    void *tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
        {
            continue;
        }

        // Check if it is IPv4
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

            std::cout << "Valid IPv4 network interface address found: " << addressBuffer << std::endl;

//            std::string address_type = "en0";
//            if (ifa->ifa_name == address_type.c_str())
//            {
//                return addressBuffer;
//            }

            // If it is not the localhost
            if (std::strncmp(addressBuffer, "127.0.0.1", sizeof(addressBuffer)) != 0)
            {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"
                return addressBuffer;
#pragma clang diagnostic pop
            }
        }
    }
    if (ifAddrStruct != NULL) freeifaddrs(ifAddrStruct);

    return nullptr;
}