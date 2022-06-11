#pragma once

#include "geo.h"
#include "domain.h"
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <deque>
#include <functional>
#include <execution>
#include <iterator>
#include <optional>
#include <map>

namespace transport_catalogue {

    struct Route_info {
        std::string_view bus_number;
        int stop_count;
        int unique_stop;
        double route_length;
        double curvature;
    };

    class TransportCatalogue {
    public:

        void AddBus(std::string_view bus_name, std::vector<std::string> &stops, bool circle);
        void AddStop(std::string_view stop_name, geo::Coordinates coordinates);

        domain::StopPtr FindStop(std::string_view stop_name) const;
        domain::BusPtr FindBus(std::string_view bus_name) const;

        void AddDistance(domain::StopPtr first_stop, domain::StopPtr second_stop, double distance);

        const std::optional<Route_info> GetRouteInfo(std::string input) const;
        const std::unordered_set<domain::BusPtr>* GetBusesForStop(std::string_view input) const;
        double GetDistance(domain::StopPtr first_stop, domain::StopPtr second_stop) const;
        const std::map<std::string_view, domain::BusPtr> GetAllRouteInfo() const;

    private:

        std::deque<domain::Stop> stops_;
        std::deque<domain::Bus> bus_;
        
        std::unordered_map<std::string_view, domain::BusPtr> busname_to_bus_;
        std::unordered_map<std::string_view, domain::StopPtr> stopname_to_stop_;
        
        std::unordered_map<domain::StopPtr, std::unordered_set<domain::BusPtr>> buses_in_stop_;
        std::unordered_map<std::pair<domain::StopPtr, domain::StopPtr>, double, domain::DistanceHasher> distances_map_;

    };
}
