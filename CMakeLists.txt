
cmake_minimum_required(VERSION 3.16)

project(SmartRouteTracker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(smart_route_tracker
    main.cpp
    backend/graph/Graph.cpp
    backend/routing/Router.cpp
)

target_include_directories(smart_route_tracker PRIVATE
    backend/graph
    backend/routing
)
