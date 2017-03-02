#include <iostream>
#include "tacopie-master/includes/tacopie/network/tcp_server.hpp"
#include "tacopie-master/includes/tacopie/network/tcp_client.hpp"

int main() {
    std::cout << "Hello, World!" << std::endl;

    tacopie::tcp_server s;
    s.start("127.0.0.1", 3001, [] (const std::shared_ptr<tacopie::tcp_client>& client) -> bool {
        std::cout << "New client" << std::endl;
        return true;
    });

    return 0;
}