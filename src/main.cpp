#include <cstring>
#include "Server/Server.h"
#include "Client/Client.h"
#include "Packets.h"

int main(int argc, char *argv[]) {
    registerPackets();

    constexpr uint16_t PORT = 6969;
    const sf::IpAddress IP = sf::IpAddress::getLocalAddress().value_or(sf::IpAddress::LocalHost);

    if (argc > 1) {
        if (strcmp(argv[1], "server") == 0) {
            Server server(IP, PORT);
            server.start();
            server.run();
        } else if (strcmp(argv[1], "client") == 0) {
            Client client(IP, PORT, argv[2]);
            client.start();
            client.run();
        }
    } else {
        auto server = Server(sf::IpAddress::LocalHost, PORT);

        std::thread serverThread([&server]() {
            server.start();
            server.run();
        });

        std::this_thread::sleep_for(std::chrono::seconds(1));

        auto client = Client(sf::IpAddress::LocalHost, PORT, "John");

        std::thread clientThread([&client]() {
            client.start();
            client.run();
        });

        if (clientThread.joinable())
            clientThread.join();

        if (serverThread.joinable())
            serverThread.join();
    }

    return 0;
}
