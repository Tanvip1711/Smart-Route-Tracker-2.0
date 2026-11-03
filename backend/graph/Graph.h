#pragma once
#include "Node.h"
#include "Edge.h"
#include <string>
#include <unordered_map>
#include <vector>

class Graph {
public:
    bool addNode(const Node& node);
    bool addRoad(const std::string& from, const std::string& to,
                 double distanceKm, double speedKmph,
                 bool bidirectional = true);
    bool setRoadBlocked(const std::string& from, const std::string& to, bool blocked);
    bool hasNode(const std::string& id) const;
    const Node* getNode(const std::string& id) const;
    const std::vector<Edge>& getEdges(const std::string& id) const;
    const std::unordered_map<std::string, Node>& getNodes() const;
    std::size_t nodeCount() const;

private:
    std::unordered_map<std::string, Node> nodes;
    std::unordered_map<std::string, std::vector<Edge>> adjacency;
};