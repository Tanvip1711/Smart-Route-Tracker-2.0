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

    std::cout << "\n===== DIJKSTRA ROUTING =====\n";
    auto dijkstraRoute = router.dijkstra("RAVET", "CHINCHWAD");
    printRoute(dijkstraRoute);

    std::cout << "\n===== A* ROUTING =====\n";
    auto astarRoute = router.astar("RAVET", "CHINCHWAD");
    printRoute(astarRoute);

    if (dijkstraRoute.found && astarRoute.found &&
        std::abs(dijkstraRoute.distanceKm - astarRoute.distanceKm) < 0.000001) {
        std::cout << "\nTEST PASSED: Both algorithms found the same distance.\n";
    } else {
        std::cout << "\nTEST FAILED: The route distances do not match.\n";
    }

    std::cout << "\n===== ROAD BLOCKING TEST =====\n";

    graph.setRoadBlocked("AKURDI", "NIGDI", true);

    auto reroute = router.dijkstra("RAVET", "CHINCHWAD");
    printRoute(reroute);

    std::cout << "\n===== A* AFTER ROAD BLOCKING =====\n";

    auto astarReroute = router.astar("RAVET", "CHINCHWAD");
    printRoute(astarReroute);

    return 0;
}