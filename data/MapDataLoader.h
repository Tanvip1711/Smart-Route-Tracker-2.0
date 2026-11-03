#pragma once

#include "../backend/graph/Graph.h"

#include <string>
#include <cstddef>

struct MapLoadStats {
    std::size_t totalFeatures = 0;
    std::size_t lineFeatures = 0;
    std::size_t loadedWays = 0;
    std::size_t skippedWays = 0;
    std::size_t estimatedSpeedEdges = 0;
};

class MapDataLoader {
public:
    bool loadGeoJson(
        const std::string& filePath,
        Graph& graph,
        MapLoadStats& stats,
        std::string& error
    ) const;

    std::string findNearestNode(
        const Graph& graph,
        double latitude,
        double longitude
    ) const;
};