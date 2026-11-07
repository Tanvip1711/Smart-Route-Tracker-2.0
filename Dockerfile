FROM ubuntu:22.04 AS build

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    nlohmann-json3-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --target route_tracker_api -j2


FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=build /app/build/route_tracker_api /app/route_tracker_api
COPY --from=build /app/data/raw/vit_bibwewadi_roads.geojson /app/data/raw/vit_bibwewadi_roads.geojson

ENV PORT=10000

EXPOSE 10000

CMD ["sh", "-c", "./route_tracker_api"]