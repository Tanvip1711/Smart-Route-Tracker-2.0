
#ifndef API_SERVER_H
#define API_SERVER_H

#include "third_party/httplib.h"
#include "../graph/Graph.h"
#include "../routing/Router.h"
#include "../../data/MapDataLoader.h"

#include <memory>
#include <string>

class ApiServer {
public:
    explicit ApiServer(int port = 8080);

    bool start();
    void stop();

private:
    int port_;
    httplib::Server server_;

    Graph graph_;
    MapDataLoader mapLoader_;
    std::unique_ptr<Router> router_;

    MapLoadStats mapStats_;
    std::string mapLoadError_;

    void configureRoutes();
    bool loadRoadNetwork();
};

#endif
