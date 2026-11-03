#include "backend/graph/Graph.h"
#include "backend/routing/Router.h"
#include <iostream>

int main() {
    Graph graph;
    graph.addNode({"RAVET", "Ravet", 18.65, 73.76});
    graph.addNode({"AKURDI", "Akurdi", 18.65, 73.77});
    graph.addNode({"NIGDI", "Nigdi", 18.65, 73.77});
    graph.addNode({"CHINCHWAD", "Chinchwad", 18.63, 73.78});

    graph.addRoad("RAVET", "AKURDI", 2.0, 30.0);
    graph.addRoad("AKURDI", "NIGDI", 1.5, 25.0);
    graph.addRoad("RAVET", "NIGDI", 4.5, 40.0);
    graph.addRoad("NIGDI", "CHINCHWAD", 2.0, 30.0);

    Router router(graph);
    auto route = router.dijkstra("RAVET", "CHINCHWAD");

    if (!route.found) {
        std::cout << "No route found.\n";
        return 1;
    }

    std::cout << "Route: ";
    for (std::size_t i = 0; i < route.path.size(); ++i) {
        if (i) std::cout << " -> ";
        std::cout << route.path[i];
    }
    std::cout << "\nDistance: " << route.distanceKm << " km\n";
    std::cout << "Estimated travel time: " << route.travelTimeMinutes << " minutes\n";

    graph.setRoadBlocked("AKURDI", "NIGDI", true);
    auto reroute = router.dijkstra("RAVET", "CHINCHWAD");
    std::cout << "After blocking AKURDI -> NIGDI: ";
    for (std::size_t i = 0; i < reroute.path.size(); ++i) {
        if (i) std::cout << " -> ";
        std::cout << reroute.path[i];
    }
    std::cout << "\n";
    return 0;
}
