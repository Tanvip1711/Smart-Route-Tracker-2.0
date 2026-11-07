#ifndef TRAFFIC_PROVIDER_H
#define TRAFFIC_PROVIDER_H

#include <nlohmann/json.hpp>

class TrafficProvider {
public:
    TrafficProvider();

    nlohmann::json getTrafficStatus() const;
};

#endif