#include "backend/graph/Graph.h"
#include "backend/routing/Router.h"
#include <iostream>
#include <cmath>

void printRoute(const RouteResult& route) {
    if (!route.found) {
        std::cout << "No route found.\n";
        return;
    }

    for (std::size_t i = 0; i < route.path.size(); ++i) {
        if (i > 0) std::cout << " -> ";
        std::cout << route.path[i];
    }

    std::cout << "\nDistance: " << route.distanceKm << " km\n";
    std::cout << "Estimated travel time: "
              << route.travelTimeMinutes << " minutes\n";
}

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

       std::cout << "\n===== SHORTEST DISTANCE ROUTE =====\n";
    auto shortest = router.dijkstra("RAVET", "CHINCHWAD");
    printRoute(shortest);

    std::cout << "\n===== FASTEST ROUTE =====\n";
    auto fastest = router.fastestRoute("RAVET", "CHINCHWAD");
    printRoute(fastest);

    std::cout << "\n===== A* ROUTING =====\n";
    auto astarRoute = router.astar("RAVET", "CHINCHWAD");
    printRoute(astarRoute);

    std::cout << "\n===== ROAD BLOCKING TEST =====\n";

    graph.setRoadBlocked("AKURDI", "NIGDI", true);

    auto reroute = router.fastestRoute("RAVET", "CHINCHWAD");
    printRoute(reroute);

    return 0;
}