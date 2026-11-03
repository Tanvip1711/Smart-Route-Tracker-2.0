#include "Graph.h"
#include <stdexcept>

bool Graph::addNode(const Node& node) {
    if (node.id.empty() || nodes.count(node.id)) return false;
    nodes.emplace(node.id, node);
    adjacency.emplace(node.id, std::vector<Edge>{});
    return true;
}

bool Graph::addRoad(const std::string& from, const std::string& to,
                    double distanceKm, double speedKmph, bool bidirectional) {
    if (!hasNode(from) || !hasNode(to) || from == to ||
        distanceKm <= 0.0 || speedKmph <= 0.0) return false;

    adjacency[from].push_back({to, distanceKm, speedKmph, false});
    if (bidirectional) {
        adjacency[to].push_back({from, distanceKm, speedKmph, false});
    }
    return true;
}

bool Graph::setRoadBlocked(const std::string& from, const std::string& to, bool blocked) {
    if (!hasNode(from) || !hasNode(to)) return false;
    bool found = false;
    for (auto& edge : adjacency[from]) {
        if (edge.to == to) {
            edge.blocked = blocked;
            found = true;
        }
    }
    // If the reverse connection exists, treat it as the other direction
    // of the same two-way road. One-way roads have no reverse edge.
    for (auto& edge : adjacency[to]) {
        if (edge.to == from) edge.blocked = blocked;
    }
    return found;
}

bool Graph::hasNode(const std::string& id) const {
    return nodes.find(id) != nodes.end();
}

const Node* Graph::getNode(const std::string& id) const {
    auto it = nodes.find(id);
    return it == nodes.end() ? nullptr : &it->second;
}

const std::vector<Edge>& Graph::getEdges(const std::string& id) const {
    static const std::vector<Edge> empty;
    auto it = adjacency.find(id);
    return it == adjacency.end() ? empty : it->second;
}

std::size_t Graph::nodeCount() const {
    return nodes.size();
}
