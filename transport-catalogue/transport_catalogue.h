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
#include <iterator>
#include <optional>
#include <map>

namespace transport_catalogue {

    struct Buses_Info {
        std::string_view route_name;
        int stop_count;
        int unique_stop;
        double route_length;
        double curvature;
    };

    namespace detail {
        struct DistanceHasher {
            size_t operator()(const std::pair<domain::StopPtr, domain::StopPtr> stops_pair) const;

        private:
            std::hash<const void *> hasher;
        };
    }

    class TransportCatalogue {
    public:

        void AddBus(std::string_view bus_name, std::vector<std::string> &stops, bool circle);
        void AddStop(std::string_view stop_name, geo::Coordinates coordinates);

        domain::StopPtr FindStop(std::string_view stop_name) const;
        domain::BusPtr FindBus(std::string_view bus_name) const;

        void SetDistanceStops(domain::StopPtr first_stop, domain::StopPtr second_stop, double distance);

        const std::optional<Buses_Info> GetBusInfo(std::string input) const;
        const std::unordered_set<domain::BusPtr>* GetStopInfo(std::string_view input) const;
        size_t GetDistance(domain::StopPtr first_stop, domain::StopPtr second_stop) const;

        const std::map<std::string_view, domain::BusPtr> GetAllBusesInfo() const;
        const std::unordered_map<std::string_view, domain::StopPtr> GetAllStop() const;

        const std::deque<domain::Stop> GetStopBase() const;
        const std::deque<domain::Bus> GetBusBase() const;
        const std::unordered_map<domain::StopPtr, std::unordered_set<domain::BusPtr>> GetBusesInStopBase() const;
        const std::unordered_map<std::pair<domain::StopPtr, domain::StopPtr>, double, detail::DistanceHasher> GetDistanceBase() const;

    private:

        std::deque<domain::Stop> stops_;
        std::deque<domain::Bus> bus_;
        
        std::unordered_map<std::string_view, domain::BusPtr> bus_binding_;
        std::unordered_map<std::string_view, domain::StopPtr> stop_binding_;
        
        std::unordered_map<domain::StopPtr, std::unordered_set<domain::BusPtr>> buses_in_stop_;
        std::unordered_map<std::pair<domain::StopPtr, domain::StopPtr>, double, detail::DistanceHasher> distance_between_stop_;

    };
}
