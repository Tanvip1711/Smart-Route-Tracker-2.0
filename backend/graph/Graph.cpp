#include "Graph.h"

bool Graph::addNode(const Node& node) {
    if (node.id.empty() || nodes.count(node.id)) {
        return false;
    }

    nodes.emplace(node.id, node);
    adjacency.emplace(node.id, std::vector<Edge>{});

    return true;
}

bool Graph::addRoad(
    const std::string& from,
    const std::string& to,
    double distanceKm,
    double speedKmph,
    bool bidirectional,
    const std::string& osmWayId,
    const std::string& roadName,
    const std::string& roadType,
    bool speedEstimated
) {
    if (!hasNode(from) || !hasNode(to) ||
        from == to || distanceKm <= 0.0 ||
        speedKmph <= 0.0) {
        return false;
    }

    Edge forward;
    forward.to = to;
    forward.distanceKm = distanceKm;
    forward.speedKmph = speedKmph;
    forward.osmWayId = osmWayId;
    forward.roadName = roadName;
    forward.roadType = roadType;
    forward.speedEstimated = speedEstimated;

    adjacency[from].push_back(forward);

    if (bidirectional) {
        Edge reverse = forward;
        reverse.to = from;
        adjacency[to].push_back(reverse);
    }

    return true;
}

bool Graph::setRoadBlocked(
    const std::string& from,
    const std::string& to,
    bool blocked
) {
    if (!hasNode(from) || !hasNode(to)) {
        return false;
    }

    bool found = false;

    for (auto& edge : adjacency[from]) {
        if (edge.to == to) {
            edge.blocked = blocked;
            found = true;
        }
    }

    for (auto& edge : adjacency[to]) {
        if (edge.to == from) {
            edge.blocked = blocked;
        }
    }

    return found;
}

bool Graph::hasNode(const std::string& id) const {
    return nodes.find(id) != nodes.end();
}

const Node* Graph::getNode(const std::string& id) const {
    auto it = nodes.find(id);

    if (it == nodes.end()) {
        return nullptr;
    }

    return &it->second;
}

const std::vector<Edge>& Graph::getEdges(
    const std::string& id
) const {
    static const std::vector<Edge> empty;

    auto it = adjacency.find(id);

    if (it == adjacency.end()) {
        return empty;
    }

    return it->second;
}

const std::unordered_map<std::string, Node>&
Graph::getNodes() const {
    return nodes;
}

std::size_t Graph::nodeCount() const {
    return nodes.size();
}

std::size_t Graph::edgeCount() const {
    std::size_t count = 0;

    for (const auto& entry : adjacency) {
        count += entry.second.size();
    }

    return count;
}