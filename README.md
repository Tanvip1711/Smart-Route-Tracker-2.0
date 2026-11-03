# Smart Route Tracker - Member 1 Starter

This is the first working C++ routing-engine milestone. The included locations and roads are TEST DATA only, not real OpenStreetMap roads.

## Compile with g++ (C++17)

```bash
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp backend/graph/Graph.cpp backend/routing/Router.cpp -o smart_route_tracker
./smart_route_tracker
```

On Windows PowerShell, run `smart_route_tracker.exe` after compilation.

## Current features
- Graph nodes and weighted road edges
- Bidirectional and one-way road creation
- Road blocking
- Dijkstra shortest-distance routing
- Distance and estimated travel-time calculation

## Next
- Add unit tests, then load real OpenStreetMap road data.
