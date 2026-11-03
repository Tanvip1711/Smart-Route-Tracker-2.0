#include "Router.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <tuple>
#include <unordered_map>

namespace {

double haversineDistance(const Node& a, const Node& b) {
    const double earthRadiusKm = 6371.0;
    const double pi = 3.14159265358979323846;

    double lat1 = a.latitude * pi / 180.0;
    double lat2 = b.latitude * pi / 180.0;
    double deltaLat = (b.latitude - a.latitude) * pi / 180.0;
    double deltaLon = (b.longitude - a.longitude) * pi / 180.0;

    double value =
        std::sin(deltaLat / 2) * std::sin(deltaLat / 2) +
        std::cos(lat1) * std::cos(lat2) *
        std::sin(deltaLon / 2) * std::sin(deltaLon / 2);

    value = std::max(0.0, std::min(1.0, value));

    return earthRadiusKm * 2 *
           std::atan2(std::sqrt(value), std::sqrt(1.0 - value));
}

}

Router::Router(const Graph& graph) : graph(graph) {}

RouteResult Router::dijkstra(
    const std::string& source,
    const std::string& destination
) const {
    return findRoute(source, destination, false, false);
}

RouteResult Router::astar(
    const std::string& source,
    const std::string& destination
) const {
    return findRoute(source, destination, true, false);
}

RouteResult Router::fastestRoute(
    const std::string& source,
    const std::string& destination
) const {
    return findRoute(source, destination, false, true);
}

double Router::heuristicScale() const {
    double scale = 1.0;

    for (const auto& [fromId, node] : graph.getNodes()) {
        for (const auto& edge : graph.getEdges(fromId)) {
            const Node* destinationNode = graph.getNode(edge.to);

            if (!destinationNode) continue;

            double straightDistance =
                haversineDistance(node, *destinationNode);

            if (straightDistance > 0.000001) {
                scale = std::min(
                    scale,
                    edge.distanceKm / straightDistance
                );
            }
        }
    }

    return std::max(0.0, scale);
}

RouteResult Router::findRoute(
    const std::string& source,
    const std::string& destination,
    bool useAStar,
    bool optimizeTime
) const {
    RouteResult result;

    if (!graph.hasNode(source) || !graph.hasNode(destination)) {
        return result;
    }

    const double infinity = std::numeric_limits<double>::infinity();
    const double scale = useAStar ? heuristicScale() : 0.0;

    auto heuristic = [&](const std::string& id) {
        if (!useAStar) return 0.0;

        const Node* current = graph.getNode(id);
        const Node* target = graph.getNode(destination);

        if (!current || !target) return 0.0;

        return scale * haversineDistance(*current, *target);
    };

    using State = std::tuple<double, double, std::string>;

    std::priority_queue<
        State,
        std::vector<State>,
        std::greater<State>
    > pq;

    std::unordered_map<std::string, double> distance;
    std::unordered_map<std::string, std::string> previous;
    std::unordered_map<std::string, Edge> previousEdge;

    distance[source] = 0.0;
    pq.push({heuristic(source), 0.0, source});

    while (!pq.empty()) {
        auto [priority, currentDistance, current] = pq.top();
        pq.pop();

        auto currentIt = distance.find(current);

        if (currentIt == distance.end() ||
            currentDistance > currentIt->second) {
            continue;
        }

        if (current == destination) break;

        for (const auto& edge : graph.getEdges(current)) {
            if (edge.blocked) continue;

            double edgeCost = optimizeTime
                ? edge.travelTimeMinutes()
                : edge.distanceKm;

            double candidate = currentDistance + edgeCost;

            auto it = distance.find(edge.to);

            if (it == distance.end() || candidate < it->second) {
                distance[edge.to] = candidate;
                previous[edge.to] = current;
                previousEdge[edge.to] = edge;

                pq.push({
                    candidate + heuristic(edge.to),
                    candidate,
                    edge.to
                });
            }
        }
    }

    if (distance.find(destination) == distance.end() ||
        !std::isfinite(distance[destination])) {
        return result;
    }

    std::string current = destination;
    result.path.push_back(current);

    while (current != source) {
        auto it = previous.find(current);

        if (it == previous.end()) {
            return RouteResult{};
        }

        const Edge& edge = previousEdge.at(current);

        result.distanceKm += edge.distanceKm;
        result.travelTimeMinutes += edge.travelTimeMinutes();

        current = it->second;
        result.path.push_back(current);
    }

    std::reverse(result.path.begin(), result.path.end());

    result.found = true;
    return result;
}