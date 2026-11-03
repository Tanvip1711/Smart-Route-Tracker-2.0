#pragma once
#include <string>

struct Edge {
    std::string to;
    double distanceKm = 0.0;
    double speedKmph = 30.0;
    bool blocked = false;

    std::string osmWayId;
    std::string roadName;
    std::string roadType;
    bool speedEstimated = false;

    double travelTimeMinutes() const {
        if (speedKmph <= 0.0) return 0.0;
        return (distanceKm / speedKmph) * 60.0;
    }
};