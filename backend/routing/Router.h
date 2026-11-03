#pragma once
#include "../graph/Graph.h"
#include <string>
#include <vector>

struct RouteResult {
    bool found = false;
    std::vector<std::string> path;
    double distanceKm = 0.0;
    double travelTimeMinutes = 0.0;
};

class Router {
public:
    explicit Router(const Graph& graph);

    RouteResult dijkstra(
        const std::string& source,
        const std::string& destination
    ) const;

    RouteResult astar(
        const std::string& source,
        const std::string& destination
    ) const;

    RouteResult fastestRoute(
        const std::string& source,
        const std::string& destination
    ) const;

private:
    const Graph& graph;

    RouteResult findRoute(
        const std::string& source,
        const std::string& destination,
        bool useAStar,
        bool optimizeTime
    ) const;

    double heuristicScale() const;
};