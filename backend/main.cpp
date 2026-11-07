#include "api/ApiServer.h"
#include <iostream>
#include <cstdlib>

int main() {

    std::cout << "Starting Smart Route Tracker 2.0...\n";

    const char* portEnv = std::getenv("PORT");
    int port = portEnv ? std::stoi(portEnv) : 8080;

    ApiServer server(port);

    if (!server.start()) {
        std::cerr << "Failed to start API server!\n";
        return 1;
    }

    return 0;
}