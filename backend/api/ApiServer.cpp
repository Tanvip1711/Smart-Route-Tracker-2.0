
#include "ApiServer.h"

#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

ApiServer::ApiServer(int port) : port_(port) {
    loadRoadNetwork();
    configureRoutes();
}

bool ApiServer::loadRoadNetwork() {

    std::cout << "\nLoading Pune road network...\n";

    if (!mapLoader_.loadGeoJson(
            "data/raw/vit_bibwewadi_roads.geojson",
            graph_,
            mapStats_,
            mapLoadError_)) {

        std::cerr << "Road network loading failed: "
                  << mapLoadError_ << "\n";

        return false;
    }

    router_ = std::make_unique<Router>(graph_);

    std::cout << "Road network loaded successfully!\n";
    std::cout << "Nodes: " << graph_.nodeCount() << "\n";
    std::cout << "Edges: " << graph_.edgeCount() << "\n";

    return true;
}

void ApiServer::configureRoutes() {

    server_.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type"}
    });

    server_.Options(R"(/.*)", [](const httplib::Request& req,
                                httplib::Response& res) {
        res.status = 204;
    });

    // Health check
    server_.Get("/health", [](const httplib::Request& req,
                              httplib::Response& res) {

        json response = {
            {"status", "ok"},
            {"service", "Smart Route Tracker 2.0"},
            {"message", "Backend API is running"}
        };

        res.set_content(response.dump(4), "application/json");
    });

    // Road network statistics
    server_.Get("/stats", [this](const httplib::Request& req,
                                 httplib::Response& res) {

        json response = {
            {"nodes", graph_.nodeCount()},
            {"edges", graph_.edgeCount()},
            {"loaded_ways", mapStats_.loadedWays},
            {"estimated_speed_edges", mapStats_.estimatedSpeedEdges},
            {"map_loaded", router_ != nullptr}
        };

        if (!mapLoadError_.empty()) {
            response["map_error"] = mapLoadError_;
        }

        res.set_content(response.dump(4), "application/json");
    });

    // Calculate route
    server_.Post("/route", [this](const httplib::Request& req,
                                  httplib::Response& res) {

        try {

            if (!router_) {
                res.status = 503;
                res.set_content(
                    json{{"error", "Road network is not loaded"},
                         {"details", mapLoadError_}}.dump(4),
                    "application/json"
                );
                return;
            }

            json body = json::parse(req.body);

            if (!body.contains("source") ||
                !body.contains("destination") ||
                !body["source"].is_object() ||
                !body["destination"].is_object()) {

                res.status = 400;
                res.set_content(
                    json{{"error", "Source and destination coordinates are required"}}.dump(4),
                    "application/json"
                );
                return;
            }

            auto validCoordinates = [](const json& location) {

                return location.contains("latitude") &&
                       location.contains("longitude") &&
                       location["latitude"].is_number() &&
                       location["longitude"].is_number() &&
                       location["latitude"].get<double>() >= -90.0 &&
                       location["latitude"].get<double>() <= 90.0 &&
                       location["longitude"].get<double>() >= -180.0 &&
                       location["longitude"].get<double>() <= 180.0;
            };

            if (!validCoordinates(body["source"]) ||
                !validCoordinates(body["destination"])) {

                res.status = 400;
                res.set_content(
                    json{{"error", "Invalid latitude or longitude"}}.dump(4),
                    "application/json"
                );
                return;
            }

            double sourceLat =
                body["source"]["latitude"].get<double>();

            double sourceLon =
                body["source"]["longitude"].get<double>();

            double destinationLat =
                body["destination"]["latitude"].get<double>();

            double destinationLon =
                body["destination"]["longitude"].get<double>();

            std::string sourceId =
                mapLoader_.findNearestNode(graph_, sourceLat, sourceLon);

            std::string destinationId =
                mapLoader_.findNearestNode(
                    graph_, destinationLat, destinationLon
                );

            if (sourceId.empty() || destinationId.empty()) {
                res.status = 404;
                res.set_content(
                    json{{"error", "Could not find nearby road nodes"}}.dump(4),
                    "application/json"
                );
                return;
            }

            std::string algorithm = "shortest";

            if (body.contains("algorithm") &&
                body["algorithm"].is_string()) {
                algorithm = body["algorithm"].get<std::string>();
            }

            RouteResult result;

            if (algorithm == "fastest") {
                result = router_->fastestRoute(sourceId, destinationId);
            }
            else if (algorithm == "astar") {
                result = router_->astar(sourceId, destinationId);
            }
            else if (algorithm == "shortest") {
                result = router_->dijkstra(sourceId, destinationId);
            }
            else {
                res.status = 400;
                res.set_content(
                    json{{"error", "Unknown algorithm. Use shortest, fastest, or astar"}}.dump(4),
                    "application/json"
                );
                return;
            }

            if (!result.found) {
                res.status = 404;
                res.set_content(
                    json{{"error", "No route found between these locations"}}.dump(4),
                    "application/json"
                );
                return;
            }

            json routeNodes = json::array();

            for (const auto& nodeId : result.path) {

                const Node* node = graph_.getNode(nodeId);

                if (node != nullptr) {
                    routeNodes.push_back({
                        {"id", node->id},
                        {"name", node->name},
                        {"latitude", node->latitude},
                        {"longitude", node->longitude}
                    });
                }
            }

            json response = {
                {"found", true},
                {"algorithm", algorithm},
                {"distance_km", result.distanceKm},
                {"travel_time_minutes", result.travelTimeMinutes},
                {"path", routeNodes},
                {"traffic_status", "unavailable"},
                {"travel_time_basis", "Road speed data; not live traffic"}
            };

            res.set_content(response.dump(4), "application/json");

        }
        catch (const json::exception& error) {

            res.status = 400;
            res.set_content(
                json{{"error", "Invalid JSON request"},
                     {"details", error.what()}}.dump(4),
                "application/json"
            );

        }
        catch (const std::exception& error) {

            res.status = 500;
            res.set_content(
                json{{"error", "Route calculation failed"},
                     {"details", error.what()}}.dump(4),
                "application/json"
            );
        }
    });

    // API welcome endpoint
    server_.Get("/", [](const httplib::Request& req,
                        httplib::Response& res) {

        json response = {
            {"project", "Smart Route Tracker 2.0"},
            {"status", "running"},
            {"health_endpoint", "/health"},
            {"stats_endpoint", "/stats"},
            {"route_endpoint", "POST /route"}
        };

        res.set_content(response.dump(4), "application/json");
    });
}

bool ApiServer::start() {

    std::cout << "\n====================================\n";
    std::cout << " Smart Route Tracker 2.0 API\n";
    std::cout << "====================================\n";

    std::cout << "Starting server on port "
              << port_ << "...\n";

    return server_.listen("0.0.0.0", port_);
}

void ApiServer::stop() {
    server_.stop();
}
