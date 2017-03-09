#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

// include the right file depending on the OS
#ifdef __WIN32__

#include <winsock2.h>

#else

#include <sys/socket.h>

#endif

#define PORT 5672
#define BLEN 512

/*
 * This function reports the error and
 * exits back to the shell:
 */
static void display_error(const char *on_what)
{
    fputs(strerror(errno), stderr);
    fputs(": ", stderr);
    fputs(on_what, stderr);
    fputc('\n', stderr);
    exit(1);
}

int main(int argc, char **argv)
{
    int data_size;
    char *address = nullptr;
    struct sockaddr_in socket_me;   // AF_INET
    struct sockaddr_in socket_them; // AF_INET
    int socket_length;              // length
    int sock;                       // Socket
    char buffer[BLEN];              // Recv buffer
    char time_date_result[BLEN];    // Date/Time Result
    time_t time_date;               // Current Time and Date
    struct tm tm;                   // Date time values

    /*
     * Use a server address from the command
     * line, if one has been provided.
     * Otherwise, this program will default
     * to using the arbitrary address
     * 127.0.0.X
     */
    if (argc >= 2)
    {
        // Addr on cmdline:
        address = argv[1];
    } else
    {
        // Use default address:
        address = (char *) "127.0.0.1";
    }

    /*
     * Create a UDP socket to use:
     */
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        display_error("socket()");
    }
    /*
     * Create a socket address, for use
     * with bind(2):
     */
    memset(&socket_me, 0, sizeof(socket_me));
    socket_me.sin_family = AF_INET;
    socket_me.sin_port = htons(PORT);
    socket_me.sin_addr.s_addr = INADDR_ANY;

    if (socket_me.sin_addr.s_addr == INADDR_NONE)
    {
        display_error("bad address.");
    }

    // socket length of our socket on bind
    socket_length = sizeof(socket_me);

    /*
     * Bind a address to our socket, so that
     * client programs can contact this
     * server:
     */
    if ((data_size = bind(sock, (struct sockaddr *) &socket_me, (socklen_t) socket_length)) == -1)
    {
        display_error("bind()");
    }

    /*
     * Now wait for requests:
     */
    for (;;)
    {
        // socket length of their socket on send and receive
        socket_length = sizeof(socket_them);

        // receive anything
        data_size = (int) recv(sock, buffer, BLEN, 0);
        // if we received something
        if (data_size < 0)
        {
            display_error("recvfrom(2)");
        }

//        data_size = (int) recvfrom(sock,                           // Our socket
//                                   buffer,                           // Receiving buffer
//                                   BLEN,                             // Buffer size
//                                   0,                                // Flags: no options
//                                   (struct sockaddr *) &socket_them, // Their socket address
//                                   (socklen_t *) &socket_length);    // Address length, in & out
//
//        std::cout << socket_them.sin_addr.s_addr << ":" << socket_them.sin_port << std::endl;

        // if we received something
        if (data_size < 0)
        {
            display_error("recvfrom(2)");
        }

        // print what we got
        std::cout << buffer << std::endl;

        /*
         * Process the request:
         */
        buffer[data_size] = 0; // null terminate
        if (!strcasecmp(buffer, "QUIT"))
        {
            break;
        }

        /*
         * Get the current date and time:
         */
        time(&time_date);            // Get current time & date
        tm = *localtime(&time_date); // Get components

        /*
         * Format a new date and time string,
         * based upon the input format string:
         */
        strftime(time_date_result,         // Formatted result
                 sizeof(time_date_result), // Max result size
                 buffer,                   // Input date/time format
                 &tm);                     // Input date/time values

        /*
         * Send the formatted result back to the
         * client program:
         */
        data_size = (int) sendto(sock,                             // Socket to send result
                                 "Hallo?",                 // The datagram result to send
                                 sizeof("Hallo?"),         // The datagram length
                                 0,                                // Flags: no options
                                 (struct sockaddr *) "145.76.241.255", // Address
                                 (socklen_t) sizeof("145.76.241.255"));       // Client address length

        // if the sending succeeded
        if (data_size < 0)
        {
            display_error("sendto(2)");
        }
    }

    /*
     * Close the socket and exit:
     */
    close(sock);
    return 0;
}