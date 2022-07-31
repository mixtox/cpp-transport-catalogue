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
        std::string name;
        geo::Coordinates coordinates;
    };

    using StopPtr = const Stop*;

    struct Bus {
        std::string bus_name;
        std::vector<StopPtr> bus_stops;
        bool is_circle;
    };

    using BusPtr = const Bus*;

    struct RouteSettings {
        double bus_velocity = 1.0;
        int bus_wait_time = 1;
    };
}
