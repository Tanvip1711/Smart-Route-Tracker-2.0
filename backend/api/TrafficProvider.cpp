#include "TrafficProvider.h"

TrafficProvider::TrafficProvider() {}

nlohmann::json TrafficProvider::getTrafficStatus() const {

    return {
        {"status", "unavailable"},
        {"data_type", "unavailable"},
        {"message", "No live traffic provider configured yet"}
    };
}