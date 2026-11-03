#include "MapDataLoader.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <limits>
#include <sstream>
#include <unordered_set>

using json = nlohmann::json;

namespace {

constexpr double PI = 3.14159265358979323846;
constexpr double EARTH_RADIUS_KM = 6371.0;

double haversine(
    double lat1, double lon1,
    double lat2, double lon2
) {
    double dLat = (lat2 - lat1) * PI / 180.0;
    double dLon = (lon2 - lon1) * PI / 180.0;

    lat1 *= PI / 180.0;
    lat2 *= PI / 180.0;

    double a =
        std::sin(dLat / 2) * std::sin(dLat / 2) +
        std::cos(lat1) * std::cos(lat2) *
        std::sin(dLon / 2) * std::sin(dLon / 2);

    return 2.0 * EARTH_RADIUS_KM *
        std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
}

std::string getTag(
    const json& properties,
    const std::string& key
) {
    if (!properties.contains(key) ||
        properties[key].is_null()) {
        return "";
    }

    if (properties[key].is_string()) {
        return properties[key].get<std::string>();
    }

    return properties[key].dump();
}

bool isRestricted(const json& properties) {
    const std::string access = getTag(properties, "access");
    const std::string motorVehicle =
        getTag(properties, "motor_vehicle");
    const std::string motorcar =
        getTag(properties, "motorcar");

    if (access == "no" || access == "private") {
        return true;
    }

    if (motorVehicle == "no" ||
        motorVehicle == "private") {
        return true;
    }

    if (motorcar == "no" ||
        motorcar == "private") {
        return true;
    }

    return false;
}

bool isMotorRoad(const std::string& highway) {
    static const std::unordered_set<std::string> allowed = {
        "motorway",
        "motorway_link",
        "trunk",
        "trunk_link",
        "primary",
        "primary_link",
        "secondary",
        "secondary_link",
        "tertiary",
        "tertiary_link",
        "unclassified",
        "residential",
        "living_street",
        "service",
        "road"
    };

    return allowed.count(highway) > 0;
}

double estimatedSpeed(const std::string& highway) {
    if (highway == "motorway") return 80.0;
    if (highway == "motorway_link") return 50.0;
    if (highway == "trunk") return 60.0;
    if (highway == "trunk_link") return 40.0;
    if (highway == "primary") return 50.0;
    if (highway == "primary_link") return 35.0;
    if (highway == "secondary") return 40.0;
    if (highway == "secondary_link") return 30.0;
    if (highway == "tertiary") return 35.0;
    if (highway == "tertiary_link") return 25.0;
    if (highway == "living_street") return 10.0;
    if (highway == "service") return 15.0;

    return 25.0;
}

double parseSpeed(const std::string& value) {
    std::string number;

    for (char c : value) {
        if (std::isdigit(
            static_cast<unsigned char>(c)
        ) || c == '.') {
            number += c;
        } else if (!number.empty()) {
            break;
        }
    }

    if (number.empty()) {
        return 0.0;
    }

    try {
        double speed = std::stod(number);

        if (value.find("mph") != std::string::npos) {
            speed *= 1.60934;
        }

        return speed;
    } catch (...) {
        return 0.0;
    }
}

std::string coordinateId(double latitude, double longitude) {
    long long lat = std::llround(latitude * 10000000.0);
    long long lon = std::llround(longitude * 10000000.0);

    return "N_" + std::to_string(lat) +
           "_" + std::to_string(lon);
}

bool getCoordinate(
    const json& coordinate,
    double& longitude,
    double& latitude
) {
    if (!coordinate.is_array() ||
        coordinate.size() < 2 ||
        !coordinate[0].is_number() ||
        !coordinate[1].is_number()) {
        return false;
    }

    longitude = coordinate[0].get<double>();
    latitude = coordinate[1].get<double>();

    return latitude >= -90.0 && latitude <= 90.0 &&
           longitude >= -180.0 && longitude <= 180.0;
}

} // namespace

bool MapDataLoader::loadGeoJson(
    const std::string& filePath,
    Graph& graph,
    MapLoadStats& stats,
    std::string& error
) const {
    std::ifstream file(filePath);

    if (!file.is_open()) {
        error = "Could not open GeoJSON file: " + filePath;
        return false;
    }

    json document;

    try {
        file >> document;
    } catch (const std::exception& e) {
        error = std::string("Invalid JSON: ") + e.what();
        return false;
    }

    if (!document.contains("features") ||
        !document["features"].is_array()) {
        error = "GeoJSON does not contain a valid features array.";
        return false;
    }

    stats = {};
    stats.totalFeatures = document["features"].size();

    for (const auto& feature : document["features"]) {
        if (!feature.contains("geometry") ||
            feature["geometry"].is_null()) {
            continue;
        }

        const auto& geometry = feature["geometry"];

        if (geometry.value("type", std::string("")) !=
            "LineString") {
            continue;
        }

        stats.lineFeatures++;

        if (!geometry.contains("coordinates") ||
            !geometry["coordinates"].is_array()) {
            stats.skippedWays++;
            continue;
        }

        json properties = json::object();

        if (feature.contains("properties") &&
            feature["properties"].is_object()) {
            properties = feature["properties"];
        }

        const std::string highway =
            getTag(properties, "highway");

        if (!isMotorRoad(highway) ||
            isRestricted(properties)) {
            stats.skippedWays++;
            continue;
        }

        const auto& coordinates = geometry["coordinates"];

        if (coordinates.size() < 2) {
            stats.skippedWays++;
            continue;
        }

        std::string wayId = getTag(properties, "@id");

        if (wayId.empty() && feature.contains("id")) {
            wayId = feature["id"].dump();
        }

        const std::string roadName =
            getTag(properties, "name");

        double speed = parseSpeed(
            getTag(properties, "maxspeed")
        );

        bool speedIsEstimated = false;

        if (speed <= 0.0) {
            speed = estimatedSpeed(highway);
            speedIsEstimated = true;
        }

        std::string oneway = getTag(properties, "oneway");

        std::transform(
            oneway.begin(), oneway.end(),
            oneway.begin(),
            [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            }
        );

        bool bidirectional = true;
        bool reverseGeometry = false;

        if (oneway == "yes" ||
            oneway == "1" ||
            oneway == "true") {
            bidirectional = false;
        } else if (oneway == "-1") {
            bidirectional = false;
            reverseGeometry = true;
        } else if (getTag(properties, "junction") ==
                   "roundabout") {
            bidirectional = false;
        }

        std::vector<std::pair<double, double>> points;

        for (const auto& coordinate : coordinates) {
            double lon, lat;

            if (!getCoordinate(coordinate, lon, lat)) {
                points.clear();
                break;
            }

            points.push_back({lat, lon});
        }

        if (points.size() < 2) {
            stats.skippedWays++;
            continue;
        }

        if (reverseGeometry) {
            std::reverse(points.begin(), points.end());
        }

        bool wayAdded = false;

        for (std::size_t i = 0; i + 1 < points.size(); i++) {
            const auto& a = points[i];
            const auto& b = points[i + 1];

            std::string fromId =
                coordinateId(a.first, a.second);

            std::string toId =
                coordinateId(b.first, b.second);

            if (!graph.hasNode(fromId)) {
                graph.addNode({
                    fromId,
                    "Road junction",
                    a.first,
                    a.second
                });
            }

            if (!graph.hasNode(toId)) {
                graph.addNode({
                    toId,
                    "Road junction",
                    b.first,
                    b.second
                });
            }

            double distance = haversine(
                a.first, a.second,
                b.first, b.second
            );

            if (distance <= 0.000001) {
                continue;
            }

            if (graph.addRoad(
                fromId,
                toId,
                distance,
                speed,
                bidirectional,
                wayId,
                roadName,
                highway,
                speedIsEstimated
            )) {
                wayAdded = true;

                if (speedIsEstimated) {
                    stats.estimatedSpeedEdges +=
                        bidirectional ? 2 : 1;
                }
            }
        }

        if (wayAdded) {
            stats.loadedWays++;
        } else {
            stats.skippedWays++;
        }
    }

    if (graph.nodeCount() == 0 ||
        graph.edgeCount() == 0) {
        error = "No routable road network was loaded. Check the GeoJSON and filters.";
        return false;
    }

    return true;
}

std::string MapDataLoader::findNearestNode(
    const Graph& graph,
    double latitude,
    double longitude
) const {
    std::string nearestId;
    double minimumDistance =
        std::numeric_limits<double>::max();

    for (const auto& entry : graph.getNodes()) {
        const Node& node = entry.second;

        double distance = haversine(
            latitude,
            longitude,
            node.latitude,
            node.longitude
        );

        if (distance < minimumDistance) {
            minimumDistance = distance;
            nearestId = node.id;
        }
    }

    return nearestId;
}