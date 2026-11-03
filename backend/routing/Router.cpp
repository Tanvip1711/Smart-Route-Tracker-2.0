#include <cmath>
#include "Router.h"
#include <algorithm>
#include <limits>
#include <queue>
#include <unordered_map>

Router::Router(const Graph& graph) : graph(graph) {}

RouteResult Router::dijkstra(const std::string& source, const std::string& destination) const {
    RouteResult result;
    if (!graph.hasNode(source) || !graph.hasNode(destination)) return result;

    using State = std::pair<double, std::string>;
    std::priority_queue<State, std::vector<State>, std::greater<State>> pq;
    std::unordered_map<std::string, double> dist;
    std::unordered_map<std::string, std::string> previous;

    for (const auto& id : {source, destination}) dist[id] = std::numeric_limits<double>::infinity();
    dist[source] = 0.0;
    pq.push({0.0, source});

    while (!pq.empty()) {
        auto [currentDistance, current] = pq.top();
        pq.pop();
        if (currentDistance != dist[current]) continue;
        if (current == destination) break;

        for (const auto& edge : graph.getEdges(current)) {
            if (edge.blocked) continue;
            double candidate = currentDistance + edge.distanceKm;
            auto it = dist.find(edge.to);
            if (it == dist.end() || candidate < it->second) {
                dist[edge.to] = candidate;
                previous[edge.to] = current;
                pq.push({candidate, edge.to});
            }
        }
    }

    auto destinationIt = dist.find(destination);
    if (destinationIt == dist.end() || !std::isfinite(destinationIt->second)) return result;

    for (std::string at = destination; ; at = previous.at(at)) {
        result.path.push_back(at);
        if (at == source) break;
    }
    std::reverse(result.path.begin(), result.path.end());
    result.distanceKm = destinationIt->second;

    for (std::size_t i = 1; i < result.path.size(); ++i) {
        const auto& edges = graph.getEdges(result.path[i - 1]);
        for (const auto& edge : edges) {
            if (edge.to == result.path[i] && !edge.blocked) {
                result.travelTimeMinutes += edge.travelTimeMinutes();
                break;
            }
        }
    }
    result.found = true;
    return result;
}
