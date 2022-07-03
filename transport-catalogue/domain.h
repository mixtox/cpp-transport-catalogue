#pragma once

#include "geo.h"
#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace domain {

    struct Stop {
        Stop() = default;

        std::string name;
        geo::Coordinates coordinates;
    };

    using StopPtr = const Stop*;

    struct Bus {
        Bus() = default;

        std::string bus_number;
        std::vector<StopPtr> bus_stops;
        bool is_circle;
    };

    using BusPtr = const Bus*;

    struct DistanceHasher {
       size_t operator()(const std::pair<domain::StopPtr, domain::StopPtr> stops_pair) const;

    private:
            std::hash<const void*> hasher;
    };
    
    struct RouteSettings {
        double bus_velocity = 1.0;
        int bus_wait_time = 1;
    };
}
