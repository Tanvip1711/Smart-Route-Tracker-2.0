#include "api/ApiServer.h"
#include <iostream>

int main() {

    std::cout << "Starting Smart Route Tracker 2.0...\n";

    ApiServer server(8080);

    if (!server.start()) {
        std::cerr << "Failed to start API server!\n";
        return 1;
    }

    return 0;
}