
#include "backend/graph/Graph.h"
#include "data/MapDataLoader.h"
#include "backend/routing/Router.h"

#include <iostream>
#include <iomanip>
#include <cmath>

void printResult(const std::string& title, const RouteResult& result) {
    std::cout << "\n===== " << title << " =====\n";

    if (!result.found) {
        std::cout << "No route found.\n";
        return;
    }

    std::cout << "Route found successfully.\n";
    std::cout << "Number of nodes in route: "
              << result.path.size() << "\n";

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Distance: "
              << result.distanceKm << " km\n";

    std::cout << "Estimated time: "
              << result.travelTimeMinutes << " minutes\n";

    if (!result.path.empty()) {
        std::cout << "Start node: " << result.path.front() << "\n";
        std::cout << "End node: " << result.path.back() << "\n";
    }
}

int main() {
    Graph graph;
    MapDataLoader loader;
    MapLoadStats stats;
    std::string error;

    std::cout << "Loading VIT Bibwewadi road network...\n";

    if (!loader.loadGeoJson(
        "data/raw/vit_bibwewadi_roads.geojson",
        graph,
        stats,
        error
    )) {
        std::cout << "ERROR: " << error << "\n";
        return 1;
    }

    std::cout << "\nGraph loaded successfully.\n";
    std::cout << "Nodes: " << graph.nodeCount() << "\n";
    std::cout << "Directed edges: " << graph.edgeCount() << "\n";

    // Find the nearest road nodes to the test coordinates.
    std::string source = loader.findNearestNode(
        graph, 18.46416, 73.867688
    );

    std::string destination = loader.findNearestNode(
        graph, 18.4707787, 73.85693055
    );

    if (source.empty() || destination.empty()) {
        std::cout << "Could not find road nodes for test coordinates.\n";
        return 1;
    }

    std::cout << "\n===== ROUTE TEST LOCATIONS =====\n";
    std::cout << "Source: " << source << "\n";
    std::cout << "Destination: " << destination << "\n";

    Router router(graph);

    RouteResult dijkstraResult =
        router.dijkstra(source, destination);

    RouteResult astarResult =
        router.astar(source, destination);

    RouteResult fastestResult =
        router.fastestRoute(source, destination);

    printResult("DIJKSTRA SHORTEST ROUTE", dijkstraResult);
    printResult("A STAR SHORTEST ROUTE", astarResult);
    printResult("FASTEST ROUTE", fastestResult);

    std::cout << "\n===== ALGORITHM COMPARISON =====\n";

    if (dijkstraResult.found && astarResult.found) {
        if (std::abs(
            dijkstraResult.distanceKm - astarResult.distanceKm
        ) < 0.000001) {
            std::cout << "PASS: Dijkstra and A* found equal distances.\n";
        } else {
            std::cout << "FAIL: Dijkstra and A* distances differ.\n";
        }
    } else {
        std::cout << "FAIL: One or both algorithms could not find a route.\n";
    }

    return 0;
}
