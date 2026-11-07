
cmake_minimum_required(VERSION 3.16)

project(SmartRouteTracker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Member 1: Standalone routing application
add_executable(smart_route_tracker
    main.cpp
    backend/graph/Graph.cpp
    backend/routing/Router.cpp
)

target_include_directories(smart_route_tracker PRIVATE
    backend/graph
    backend/routing
)

# Member 2: Backend HTTP API + routing integration
add_executable(route_tracker_api
    backend/main.cpp
    backend/api/ApiServer.cpp
    backend/api/TrafficProvider.cpp
    backend/graph/Graph.cpp
    backend/routing/Router.cpp
    data/MapDataLoader.cpp
)

target_include_directories(route_tracker_api PRIVATE
    backend/api
    backend/graph
    backend/routing
    data
)

# Windows networking library required by httplib
target_link_libraries(route_tracker_api PRIVATE ws2_32)
