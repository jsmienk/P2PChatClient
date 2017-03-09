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

// include the right file depending on the OS
#ifdef __WIN32__
#include <winsock2.h>
#else

#include <sys/socket.h>

#endif

// port name == LoRa
#define PORT 5672
#define BLEN 512
#define SUBNET "145.76.241"
#define MASK "255"

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
    exit(1);
}

int sock;                       // socket
struct sockaddr_in socket_me;   // our socket address
struct sockaddr_in socket_them; // the socket address for the sender
int data_size;

void receiving()
{
    // Wait for incoming messages
    // 2 types:
    //  - New node in the network
    //  - Onion we have to redirect
    char buffer[BLEN];              // receive buffer

    for (;;)
    {
        // Receive anything
        data_size = (int) recv(sock, buffer, BLEN, 0);
        // If we received something
        if (data_size < 0)
        {
            display_error("recvfrom(2)");
        }

        // Print what we got
        std::cout << "Received: " << buffer << std::endl;

//        // Process
//        buffer[data_size] = 0; // null terminate
//        if (!strcasecmp(buffer, "QUIT"))
//        {
//            break;
//        }
//
//        /*
//         * Get the current date and time:
//         */
//        time(&time_date_current);            // Get current time & date
//        tm = *localtime(&time_date_current); // Get components
//
//        /*
//         * Format a new date and time string,
//         * based upon the input format string:
//         */
//        strftime(time_date_result,         // Formatted result
//                 sizeof(time_date_result), // Max result size
//                 buffer,                   // Input date/time format
//                 &tm);                     // Input date/time values
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
    }
}

void sending()
{
    for (int i = 0; i < 100; i++)
    {
        data_size = (int) sendto(sock, std::to_string(i).c_str(), sizeof(i), 0, (struct sockaddr *) &socket_them,
                                 sizeof(socket_them));
    }
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
    socket_them.sin_addr.s_addr = inet_addr("127.0.0.1");//SUBNET "." MASK);
    if (socket_them.sin_addr.s_addr == INADDR_NONE)
    {
        display_error("bad address");
    }

    // Broadcast my IP, Kp and request other IPs and Kps (and nickname if client)
    char *broadcast_message = (char *) "Hallo!";
    data_size = (int) sendto(sock, broadcast_message, sizeof(broadcast_message), 0, (struct sockaddr *) &socket_them,
                             sizeof(socket_them));

    // Start the thread that will receive messages
    std::thread thread_receive(receiving);
    // Start the thread that will send messages
    std::thread thread_send(sending);

    thread_receive.join();
    std::cout << "Receive thread started." << std::endl;
    thread_send.join();
    std::cout << "Send thread started." << std::endl;

    // TEST BROADCAST "Hallo!"
    data_size = (int) sendto(sock, broadcast_message, sizeof(broadcast_message), 0, (struct sockaddr *) &socket_them,
                             sizeof(socket_them));
    std::cout << "TETS BROADCAST SEND" << std::endl;

    // Close the socket and exit
    close(sock);
    return 0;
}