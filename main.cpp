#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <iostream>
#include <thread>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "cryptopp565/aes.h"
#include "cryptopp565/modes.h"
#include "cryptopp565/filters.h"
#include "cryptopp565/osrng.h"
#include "cryptopp565/rsa.h"
#include "rapidjson/error/en.h"
#include <cctype>

#ifdef __WIN32__

#include <winsock2.h>
#include <wininet.h>
#include <ws2tcpip.h>
#include <sstream>

#else

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <sstream>

#endif

// port name == LoRa
#define PORT 5672
#define BLEN 1024
#define MASK "255"
#define RSALEN 2048

// pre-declarations
static std::string get_network_interface_address();

static void display_error(const char *on_what);
std::string peel(std::string cipher);
void generateKeys();
void broadCast(std::string data);

int sock;                       // socket
struct sockaddr_in socket_me;   // our socket address
struct sockaddr_in socket_them; // the socket address for the sender

int data_size;

std::string address = get_network_interface_address();
std::string address_broadcast;
std::string last_send_message;

CryptoPP::RSA::PrivateKey privateKey;
CryptoPP::RSA::PublicKey publicKey;
CryptoPP::InvertibleRSAFunction params;
CryptoPP::AutoSeededRandomPool rng;

void receiving() {
    std::cout << "Receive thread started." << std::endl;

    // Wait for incoming messages
    // 3 types:
    //  - Response on initial broadcast
    //  - New node in the network
    //  - Onion we have to redirect
    char buffer[BLEN];              // receive buffer

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    for (;;) {
        memset(buffer, 0, sizeof(buffer));

        // Receive anything
        data_size = (int) recv(sock, buffer, BLEN, 0);
        // If we received something
        if (data_size < 0) {
            display_error("recvfrom(2)");
        }

        std::string received = buffer;

        // Print what we got

        std::cout << "Received: " << received << std::endl;

        rapidjson::Document json;

        // Check if parse succeeded
        try {
            if (json.Parse<0>(received.c_str()).HasParseError()) {
                        display_error("JSON could not be parsed!");
            } else {
                json.Parse(received.c_str());

                if (json.HasMember("Data")) {
                    assert(json["Data"].IsString());
                    std::string decrypted_message = peel(json["Data"].GetString());
                    if(decrypted_message != "0"){
                        std::string new_message = "{\"NewData\":\"" + decrypted_message + "\"}";

                        broadCast(new_message);
                    }
                } else if (json.HasMember("MessageType")) {
                    //doe iets
                }
            }
        } catch (std::exception) {

        }
    }
}

#pragma clang diagnostic pop


void broadCast(std::string data){
    std::cout << "broadcast decrypted message" << std::endl;

    data_size = (int) sendto(sock, data.c_str(), data.size(), 0,
                             (struct sockaddr *) &socket_them,
                             sizeof(socket_them));
    last_send_message = data;
}

void sending() {
    std::cout << "Send thread started." << std::endl;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    for (;;) {
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

int main(int argc, char **argv) {

#ifdef __WIN32__
    WSADATA Data;
    WSAStartup(MAKEWORD(2, 2), &Data);
#endif
    // Prepare socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        display_error("socket()");
    }
    std::cout << "Socket prepared." << std::endl;

    // Create a socket address, for use with bind(2)
    memset(&socket_me, 0, sizeof(socket_me));
    socket_me.sin_family = AF_INET;
    socket_me.sin_port = htons(PORT);
    socket_me.sin_addr.s_addr = INADDR_ANY;

    generateKeys();

    // Bind a address to our socket, so that client programs can contact this node
    if ((data_size = bind(sock, (struct sockaddr *) &socket_me, (socklen_t) sizeof(socket_me))) == -1) {
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
    if (socket_them.sin_addr.s_addr == INADDR_NONE) {
        display_error("bad address");
    }

    // Allow broadcasts
    if ((data_size = setsockopt(sock,
                                SOL_SOCKET,
                                SO_BROADCAST,
                                (const char *) &socket_them,
                                sizeof(socket_them))) == -1) {
        display_error("setsockopt(SO_BROADCAST)");
    }

    // Broadcast my IP, Kp and request other IPs and Kps (and nickname if client)

    std::ostringstream osMod;
    std::ostringstream osExp;
    osMod << publicKey.GetModulus();
    osExp << publicKey.GetPublicExponent();


    //Broadcast Modulus and PublicExponent
    std::string broadcast_message = "{\"Modulus\": \"" + osMod.str().substr(0, osMod.str().size()-1) + "\", \"PublicExponent\":\"" + osExp.str().substr(0, osExp.str().size()-1) + "\"}";

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
#ifdef __WIN32__
    WSACleanup();
#endif
    return 0;
}

/**
 * Displays the error and reports back to the shell
 * @param on_what error message
 */
static void display_error(const char *on_what) {
    fputs(strerror(errno), stderr);
    fputs(": ", stderr);
    fputs(on_what, stderr);
    fputc('\n', stderr);
    throw std::exception();
}

void generateKeys(){
    params.GenerateRandomWithKeySize(rng, RSALEN);
    privateKey = CryptoPP::RSA::PrivateKey(params);
    publicKey = CryptoPP::RSA::PublicKey(params);
//    cout << "modulus: " << publicKey.GetModulus() << endl;
//    cout << "public exponent: " << publicKey.GetPublicExponent() << endl;
}

std::string peel(std::string cipher) { //decrypt
//std::string peel(std::string plain) { //decrypt
    std::string recovered;
//    std::string plain= "Hallo tekst";

    //Encrypt
//    std::string plain="RSA Encryption", cipher, recovered;
//    std::string cipher;
//    CryptoPP::RSAES_OAEP_SHA_Encryptor e(publicKey);
//    CryptoPP::StringSource ss1(plain, true,
//                     new CryptoPP::PK_EncryptorFilter(rng, e,
//                                            new CryptoPP::StringSink(cipher)
//                     ) // PK_EncryptorFilter
//    ); // StringSource

//    cout << "plaintext: " << plain << endl;
//    std::cout << "cipher: " << cipher << std::endl;

    //Decrypt
    bool contains_non_alpha = cipher.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") != std::string::npos;
    if(contains_non_alpha){
        CryptoPP::RSAES_OAEP_SHA_Decryptor d(privateKey);
        CryptoPP::StringSource ss2(cipher,true, new CryptoPP::PK_DecryptorFilter(rng, d, new CryptoPP::StringSink(recovered))); // StringSource
        return recovered;
    }else{
        std::cerr << "Invalid CipherText" << std::endl;
        return "0";
    }
}

/**
 * Get a IPv4 network interface address
 * @return address or nullptr
 */

#ifdef __WIN32__

static std::string get_network_interface_address() {
    WSADATA WSAData;

    // Initialize winsock dll
    if (::WSAStartup(MAKEWORD(1, 0), &WSAData)) {
        // Error handling
        display_error("WSAStartup error");
    }

    // Get local host name
    char szHostName[128] = "";

    if (::gethostname(szHostName, sizeof(szHostName))) {
        display_error("WSA Error no hostNames");
        WSAGetLastError();
        // Error handling -> call 'WSAGetLastError()'
    }

    // Get local IP addresses
    struct sockaddr_in SocketAddress;
    struct hostent *pHost = 0;

    pHost = ::gethostbyname(szHostName);
    if (!pHost) {
        display_error("WSA Error no pHosts");
        WSAGetLastError();
        // Error handling -> call 'WSAGetLastError()'
    }

    char aszIPAddresses[10][16];

    int ipCount = -1;

    for (int iCnt = 0; ((pHost->h_addr_list[iCnt]) && (iCnt < 10)); ++iCnt) {
        memcpy(&SocketAddress.sin_addr, pHost->h_addr_list[iCnt], pHost->h_length);
        strcpy(aszIPAddresses[iCnt], inet_ntoa(SocketAddress.sin_addr));
        ipCount++;
        std::cout << "No. " << ipCount << " - IP Adress: " << aszIPAddresses[ipCount] << std::endl;

    }

    WSACleanup();

    std::cout << "Pick your network adapter" << std::endl;
    int adapter;
    std::cin >> adapter;

    return aszIPAddresses[adapter];
}

#else

static std::string get_network_interface_address() {
    struct ifaddrs *ifAddrStruct = NULL;
    struct ifaddrs *ifa = NULL;
    void *tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);


    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }

        // Check if it is IPv4
        if (ifa->ifa_addr->sa_family == AF_INET) {
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
            if (std::strncmp(addressBuffer, "127.0.0.1", sizeof(addressBuffer)) != 0) {
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

#endif